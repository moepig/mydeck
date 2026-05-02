# テスト実行方法（アプリ）

## 使用フレームワーク

- **NUnit 4.1.0**
- **NSubstitute 5.1.0**（モック）

---

## テストの実行

```bash
cd win
dotnet test
```

詳細ログあり:

```bash
dotnet test --logger "console;verbosity=detailed"
```

特定のテストクラス・メソッドを絞り込む場合:

```bash
dotnet test --filter "FullyQualifiedName~CommandDispatcher"
```

---

## テスト対象

| ディレクトリ | テストクラス | 対象 |
|---|---|---|
| `Command/` | `CommandDispatcherTests` | ボタンイベントとコマンドのディスパッチ処理 |
| `Config/` | `ConfigLoaderTests` | JSON 設定ファイルの読み書き |
| `Hid/` | `HidReportTests` | HID レポートのパース処理 |

## テスト対象外

以下のクラスは実デバイスや GUI 環境に依存するためテスト対象外です。
`IHidDevice` インターフェースをモックすることで間接的に検証します。

- `HidDevice` — 実 USB デバイスが必要
- `TrayIconManager` — Windows Forms 環境が必要

---

## コードカバレッジ（任意）

```bash
dotnet test --collect:"XPlat Code Coverage"
```

HTML レポートを生成する場合:

```bash
dotnet tool install -g dotnet-reportgenerator-globaltool
reportgenerator -reports:"**/coverage.cobertura.xml" -targetdir:"coverage-report" -reporttypes:Html
```

`coverage-report/index.html` をブラウザで開くと確認できます。
