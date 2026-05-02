# ローカルでの実行方法

## 前提条件

- .NET SDK 10.0 以上
- MyDeck デバイス（USB HID）が接続されていることが望ましいが、未接続でも起動は可能
  - 未接続の場合、一定間隔で自動再接続を試みます

---

## dotnet run で実行

```bash
cd win
dotnet run --project MyDeck/MyDeck.csproj
```

## デバッグビルドの実行ファイルを直接起動

```bash
cd win
dotnet build MyDeck.sln -c Debug
./MyDeck/bin/Debug/net10.0-windows/MyDeck.exe
```

---

## 設定画面を直接開いて起動

`--settings` 引数を渡すと、起動直後にボタンコマンド設定画面が開きます。

```bash
dotnet run --project MyDeck/MyDeck.csproj -- --settings
```

または実行ファイルに直接引数を渡す場合:

```bash
./MyDeck/bin/Debug/net10.0-windows/MyDeck.exe --settings
```

---

## 動作

- 起動するとシステムトレイにアイコンが表示されます
- トレイアイコンを右クリック → **設定** で設定画面を開けます
- 設定ファイルは `%AppData%\MyDeck\mappings.json` に保存されます
  - 初回起動時に自動で作成されます

---

## アンインストーラーをビルドせずに動作確認したい場合

インストーラーを使わずにローカルで使い続ける場合、自動起動レジストリは設定されません。
必要であれば手動で登録してください:

```powershell
$exe = Resolve-Path "win/MyDeck/bin/Debug/net10.0-windows/MyDeck.exe"
Set-ItemProperty `
  -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" `
  -Name "MyDeck" `
  -Value $exe
```
