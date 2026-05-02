# テスト実行方法（インストーラー）

## 使用フレームワーク

- **Pester 5.0.0+**（PowerShell テストフレームワーク）

テストは **Static** と **Dynamic** の 2 種類に分かれます。

---

## 前提条件

### Pester のインストール

```powershell
Install-Module Pester -Force -SkipPublisherCheck -MinimumVersion 5.0.0
```

### MSI のビルド

テスト実行前に MSI をビルドしておく必要があります。
ビルド手順は [build.md](build.md) を参照してください。
MSI は `win/artifacts/installer/MyDeck-Setup.msi` に配置されている必要があります。

---

## Static テスト（MSI プロパティ検証）

管理者権限・Docker 不要で実行できます。

```powershell
cd win
Invoke-Pester MyDeck.Installer.Tests/Install.Tests.ps1 -Tag Static -Output Detailed
```

### 検証内容

- MSI ファイルが存在すること
- ProductName = `MyDeck`
- Manufacturer = `MyDeck`
- ProductVersion が `x.y.z` 形式であること
- ProductCode・UpgradeCode が GUID 形式であること

---

## Dynamic テスト（実インストール検証）

Windows コンテナー内で実際にインストール・アンインストールを行います。
ローカル環境には影響しません。

### 追加の前提条件

- Docker Desktop がインストール済みで起動していること
- **Windows コンテナーモードに切り替えていること**
  （Docker タスクトレイアイコンを右クリック → "Switch to Windows containers"）

### 実行

```powershell
cd win/MyDeck.Installer.Tests
.\Run-DynamicTests.ps1
```

MSI パスを明示する場合:

```powershell
.\Run-DynamicTests.ps1 -MsiPath "C:\path\to\MyDeck-Setup.msi"
```

### テスト内容

**インストール後:**
- `%ProgramFiles%\MyDeck\MyDeck.exe` が存在すること
- インストールディレクトリが存在すること
- スタートメニューのショートカットが存在すること
- `HKCU\...\Run\MyDeck` レジストリエントリが存在すること

**アンインストール後:**
- `MyDeck.exe` が削除されていること
- インストールディレクトリが削除されていること
- スタートメニューのショートカットが削除されていること
- レジストリエントリが削除されていること

### 使用イメージ

```
mcr.microsoft.com/powershell:windowsservercore-ltsc2022
```

初回実行時に自動で pull されます。
