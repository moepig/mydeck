# Windows 側テスト

## 前提条件

- [.NET 10 SDK](https://dotnet.microsoft.com/download) がインストール済み

```bash
dotnet --version  # 10.0.x であること
```

---

## ユニットテスト（NUnit）

```bash
cd src/mydeck/win

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

## インストーラーテスト（Pester）

### 前提条件：インストーラーのビルド

```powershell
cd src/mydeck/win

dotnet publish MyDeck/MyDeck.csproj `
    -c Release -r win-x64 --self-contained -o artifacts/publish

dotnet build MyDeck.Installer/MyDeck.Installer.wixproj `
    -c Release `
    -p:PublishDir=$(Resolve-Path artifacts/publish)

New-Item -ItemType Directory -Force -Path artifacts/installer
Copy-Item MyDeck.Installer/bin/Release/MyDeck-Setup.msi artifacts/installer/
```

### 静的テスト（MSI プロパティ検証）

管理者権限・Docker 不要。

```powershell
cd src/mydeck/win

Install-Module Pester -Force -SkipPublisherCheck -MinimumVersion 5.0.0

Invoke-Pester MyDeck.Installer.Tests/Install.Tests.ps1 -Tag Static -Output Detailed
```

### 動的テスト（実インストール・アンインストール）

Windows コンテナ内で実行するためローカル環境に影響しない。

**前提条件：**
- Docker Desktop がインストール済みで起動していること
- Windows コンテナモードに切り替え済みであること
  - Docker Desktop タスクトレイアイコンを右クリック → *Switch to Windows containers*

```powershell
cd src/mydeck/win/MyDeck.Installer.Tests

# MSI は自動検索
.\Run-DynamicTests.ps1

# MSI パスを明示する場合
.\Run-DynamicTests.ps1 -MsiPath "C:\path\to\MyDeck-Setup.msi"
```
