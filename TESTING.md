# テスト方法

## 概要

| # | テスト種別 | 対象 | フレームワーク | 実行環境 |
|---|-----------|------|--------------|---------|
| 1 | ユニットテスト | Pro Micro ファームウェア | Unity (PlatformIO) | Linux / macOS / Windows |
| 2 | ユニットテスト | Windows C# アプリ | NUnit | Linux / macOS / Windows |
| 3 | 静的インストーラーテスト | MSI プロパティ検証 | Pester | Windows |
| 4 | 動的インストーラーテスト | 実インストール・アンインストール | Pester | Windows コンテナ（Docker） |
| 5 | CI | 全テストの自動実行 | GitHub Actions | ubuntu-latest / windows-latest |

---

## 1. Pro Micro ユニットテスト

ハードウェア不要。PC 上の `native` 環境で実行する。

### 前提条件

```bash
pip install platformio
```

### 実行

```bash
cd board

# 全テスト
pio test -e native

# 詳細出力
pio test -e native -v
```

## 2. Windows C# ユニットテスト

### 前提条件

```bash
dotnet --version   # 10.0.x であること
```

### 実行

```bash
cd win

dotnet test --logger "console;verbosity=detailed"
```

### カバレッジ計測（任意）

```bash
dotnet test --collect:"XPlat Code Coverage"

dotnet tool install -g dotnet-reportgenerator-globaltool
reportgenerator -reports:"**/coverage.cobertura.xml" -targetdir:"coverage-report" -reporttypes:Html
# coverage-report/index.html をブラウザで開く
```

### ユニットテスト対象外

| クラス | 理由 |
|-------|------|
| `HidDevice.cs` | 実デバイスとの USB 通信が必要。`IHidDevice` モックで代替 |
| `TrayIconManager.cs` | Windows Forms GUI 環境が必要 |

---

## 3 & 4. インストーラーテスト

### 前提条件（共通）

インストーラーを事前にビルドしておく。

```powershell
cd win

dotnet publish MyDeck/MyDeck.csproj `
    -c Release -r win-x64 --self-contained -o artifacts/publish

dotnet build MyDeck.Installer/MyDeck.Installer.wixproj `
    -c Release `
    -p:PublishDir=$(Resolve-Path artifacts/publish)

New-Item -ItemType Directory -Force -Path artifacts/installer
Copy-Item MyDeck.Installer/bin/Release/MyDeck-Setup.msi artifacts/installer/
```

---

### 3. 静的テスト（MSI プロパティ検証）

MSI をインストールせずに内容を検証する。管理者権限・Docker 不要。

```powershell
cd win

Install-Module Pester -Force -SkipPublisherCheck -MinimumVersion 5.0.0

Invoke-Pester MyDeck.Installer.Tests/Install.Tests.ps1 -Tag Static -Output Detailed
```

#### 検証内容

| テスト | 内容 |
|-------|------|
| MSI ファイルの存在 | `artifacts/installer/MyDeck-Setup.msi` が存在する |
| ProductName | `MyDeck` である |
| Manufacturer | `MyDeck` である |
| ProductVersion | semver 形式（`x.y.z`）である |
| ProductCode | GUID 形式である |
| UpgradeCode | GUID 形式である |

---

### 4. 動的テスト（実インストール・アンインストール）

**Windows コンテナ内で実行する。ローカル環境には影響しない。**

#### 前提条件

- Docker Desktop がインストール済みで起動していること
- **Windows コンテナモード**に切り替え済みであること
  - Docker Desktop タスクトレイアイコンを右クリック → *Switch to Windows containers*

#### 実行（管理者権限不要）

```powershell
cd win/MyDeck.Installer.Tests

# MSI は自動検索
.\Run-DynamicTests.ps1

# MSI パスを明示
.\Run-DynamicTests.ps1 -MsiPath "C:\path\to\MyDeck-Setup.msi"
```

#### スクリプトの処理フロー

```
Run-DynamicTests.ps1
    │
    ├─ Docker 起動確認
    ├─ Windows コンテナモード確認
    ├─ MSI パス解決
    ├─ イメージ pull（mcr.microsoft.com/powershell:windowsservercore-ltsc2022）
    │
    └─ docker run --rm
          -v <MSIディレクトリ>  → C:\installer
          -v <テストスクリプト> → C:\tests
          -e MSI_PATH=C:\installer\MyDeck-Setup.msi
          │
          └─ コンテナ内: Pester -Tag Dynamic
                  インストール → ファイル/レジストリ確認
                  アンインストール → クリーンアップ確認
          │
          └─ --rm によりコンテナ自動削除
```

#### 検証内容

| フェーズ | テスト |
|---------|-------|
| インストール | `msiexec` 終了コード 0 |
| インストール | `%ProgramFiles%\MyDeck\MyDeck.exe` が存在する |
| インストール | スタートメニューショートカットが存在する |
| インストール | `HKCU\...\Run\MyDeck` レジストリエントリが存在する |
| アンインストール | `msiexec` 終了コード 0 |
| アンインストール | インストール先ディレクトリが削除されている |
| アンインストール | スタートメニューショートカットが削除されている |
| アンインストール | レジストリエントリが削除されている |

---

## 5. CI（GitHub Actions）

`main` / `master` への push および PR で自動実行される。

```
.github/workflows/test.yml
```

### ジョブ構成

```
promicro              (ubuntu-latest)
│  └─ pio test -e native
│
win                   (ubuntu-latest)
│  └─ dotnet test
│
└─→ installer         (windows-latest)  ← win が通過後に実行
       ├─ dotnet publish
       ├─ WiX ビルド → MSI 生成
       ├─ artifact アップロード（保持 30 日）
       ├─ Pester -Tag Static
       └─ Pester -Tag Dynamic（$env:MSI_PATH で MSI パスを注入）
```

### 成果物

| 名前 | 内容 | 保持期間 |
|------|------|---------|
| `MyDeck-Setup` | `MyDeck-Setup.msi` | 30 日 |
