# ビルド方法

## 前提条件

- .NET SDK 10.0 以上
- WiX Toolset は NuGet 経由で自動取得されるため、別途インストール不要

---

## アプリのビルド

```bash
cd win
dotnet build MyDeck.sln
```

リリースビルド:

```bash
cd win
dotnet build MyDeck.sln -c Release
```

---

## インストーラーのビルド

インストーラーのビルドは **2 ステップ** に分かれます。

### ステップ 1: アプリを発行 (publish)

```bash
cd win
dotnet publish MyDeck/MyDeck.csproj -p:PublishProfile=installer
```

発行物は `win/artifacts/publish/` に出力されます。

### ステップ 2: MSI をビルド

```bash
cd win
dotnet build MyDeck.Installer/MyDeck.Installer.wixproj -c Release
```

完成した MSI は `win/MyDeck.Installer/bin/Release/MyDeck-Setup.msi` に生成されます。

`artifacts/installer/` へコピーする場合:

```powershell
New-Item -ItemType Directory -Force -Path artifacts/installer
Copy-Item MyDeck.Installer/bin/Release/MyDeck-Setup.msi artifacts/installer/
```

### CI での発行ディレクトリ上書き

CI 環境など、発行先を明示したい場合は `-p:PublishDir` で絶対パスを渡します。

```bash
dotnet build MyDeck.Installer/MyDeck.Installer.wixproj -c Release \
  -p:PublishDir="C:/path/to/publish/"
```

---

## 発行プロファイルの詳細

`MyDeck/Properties/PublishProfiles/installer.pubxml` の設定:

| 項目 | 値 |
|---|---|
| Configuration | Release |
| Runtime | win-x64 |
| SelfContained | true |
| PublishReadyToRun | true |
| 出力先 | `../../artifacts/publish/` |
