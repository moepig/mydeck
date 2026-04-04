# VSCode + PlatformIO 拡張での操作

## セットアップ

1. VSCode に [PlatformIO IDE 拡張](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) をインストールする
2. VSCode で `board/` フォルダを開く — `platformio.ini` が自動検出され、PlatformIO が有効になる
3. 初回はツールチェーンの自動ダウンロードが走るため、完了まで待つ（ステータスバーに進捗が表示される）
4. UIAPduino 環境を使う場合は、`board/` で以下を実行してビルドスクリプトを生成する（`.gitignore` 済みのため手動生成が必要）

   ```bash
   python scripts/fetch_build_rv003usb.py
   ```

   生成されない場合、`uiapduino` / `uiapduino_debug` のビルド時に `missing SConscript file 'pio_build_rv003usb.py'` エラーになる。

> ワークスペースを開くと推奨拡張のインストールが提案される。

## ビルド

ステータスバー左下の PlatformIO ツールバー、またはコマンドパレットから実行する。

| 操作 | ステータスバー | コマンドパレット (`Ctrl+Shift+P`) |
|------|--------------|--------------------------------|
| ビルド | ✓ (チェックマーク) | `PlatformIO: Build` |
| 書き込み | → (矢印) | `PlatformIO: Upload` |
| テスト | フラスコアイコン | `PlatformIO: Test` |
| シリアルモニタ | コンセントアイコン | `PlatformIO: Serial Monitor` |
| クリーン | — | `PlatformIO: Clean` |

## 環境の切り替え

`platformio.ini` に複数の環境が定義されている。ステータスバーの環境名をクリックして切り替える。

| 環境 | 用途 |
|------|------|
| `sparkfun_promicro16` | Pro Micro ビルド・書き込み（デフォルト） |
| `uiapduino` | UIAPduino ビルド・書き込み |
| `uiapduino_debug` | UIAPduino デバッグビルド（SWIO printf 有効） |
| `native` | ユニットテスト（PC 上で実行） |

## ユニットテスト

1. ステータスバーで環境を `native` に切り替える
2. テストアイコン (フラスコ) をクリック、またはコマンドパレットで `PlatformIO: Test` を実行

サイドバーの PlatformIO アイコン → Project Tasks → native → Test からも実行できる。

## UIAPduino の書き込み

1. ステータスバーで環境を `uiapduino` に切り替える
2. ボードを書き込み待機モードにする（[手順](flash_procedure.md#書き込み待機モードへの切り替え)）
3. Upload ボタン (→) をクリック

デバッグビルドを書き込む場合は環境を `uiapduino_debug` に切り替える。

## Pro Micro の書き込み

1. ステータスバーで環境を `sparkfun_promicro16` に切り替える
2. Upload ボタン (→) をクリック
3. 書き込みに失敗する場合は RST ピンをダブルタップしてブートローダを起動する（[手順](flash_pio_promicro.md#ブートローダ起動書き込みに失敗する場合)）

## IntelliSense / コード補完

PlatformIO 拡張が自動で `c_cpp_properties.json` を生成し、インクルードパスやマクロを設定する。
環境を切り替えると IntelliSense の対象も連動して変わる。

赤波線が消えない場合はコマンドパレットで `PlatformIO: Rebuild IntelliSense Index` を実行する。
