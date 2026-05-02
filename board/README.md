# board

マクロパッドのファームウェア。PlatformIO でビルド・テスト・書き込みを行う。

## ディレクトリ構成

```
board/
├── platformio.ini          PlatformIO プロジェクト設定（環境・ビルドフラグ）
├── Makefile                pio コマンドのショートカット (make build / test / flash)
├── src/                    ファームウェアソースコード
│   ├── mydeck.cpp              Pro Micro エントリポイント (Arduino)
│   ├── domain/                 ドメイン層 — ボード非依存のビジネスロジック
│   │   ├── button_service.{h,c}     ボタンスキャン・デバウンス・イベント生成
│   │   ├── led_service.{h,c}        LED 点灯パターン制御
│   │   ├── hid_report.h             HID レポート構造体定義
│   │   ├── button_port.h            ボタン読み取りインターフェース
│   │   └── led_port.h               LED 制御インターフェース
│   ├── infrastructure/         インフラ層 — ハードウェア固有の Port 実装
│   │   ├── arduino_button_port.h    Arduino (ATmega32U4) ボタン実装
│   │   ├── arduino_led_port.h       Arduino LED 実装
│   │   ├── ch32_button_port.h       CH32V003 ボタン実装 (GPIOC 直接操作)
│   │   └── ch32_led_port.h          CH32V003 LED 実装
│   └── boards/                 ボード固有の設定・エントリポイント
│       ├── promicro/
│       │   └── board_config.h       Pro Micro ピン定義 (4x4 マトリクス)
│       └── uiapduino/
│           ├── board_config.h       UIAPduino ピン定義 (3x5 マトリクス)
│           ├── main.c               UIAPduino エントリポイント (ch32fun)
│           ├── usb_config.h         rv003usb USB ディスクリプタ定義
│           └── Makefile             ch32fun 単体ビルド用 (PlatformIO 不要)
├── test/                   ユニットテスト (Unity / PlatformIO native)
│   ├── test_button_service/
│   ├── test_led_service/
│   └── test_hid_report/
├── ext_src/                外部依存 (ch32fun, rv003usb) — .gitignore 済み
├── .vscode/                VSCode ワークスペース設定
│   └── extensions.json         推奨拡張 (PlatformIO IDE)
└── docs/                   設計・手順ドキュメント
    ├── flash_procedure.md      書き込み手順の目次・共通リファレンス
    ├── flash_pio_promicro.md   Pro Micro — PlatformIO で書き込み
    ├── flash_pio_uiapduino.md  UIAPduino — PlatformIO で書き込み
    ├── flash_make_uiapduino.md UIAPduino — Makefile (ch32fun) で書き込み
    ├── pin_assignment.md       ピンアサイン・マトリクス配線図
    ├── testing.md              テスト実行手順
    └── vscode_platformio.md    VSCode + PlatformIO 拡張での操作
```

## 対応ボード

| ボード | MCU | マトリクス | PlatformIO env |
|--------|-----|-----------|----------------|
| SparkFun Pro Micro | ATmega32U4 | 2x6 (12 ボタン) | `sparkfun_promicro16` |
| UIAPduino Pro Micro | CH32V003 (RISC-V) | 2x6 (12 ボタン) | `uiapduino` |

## クイックスタート

```powershell
cd board

# ユニットテスト
pio test -e native

# Pro Micro ビルド・書き込み
pio run -e sparkfun_promicro16
pio run -e sparkfun_promicro16 --target upload

# UIAPduino ビルド・書き込み
pio run -e uiapduino
pio run -e uiapduino --target upload
```

詳細は [docs/flash_procedure.md](docs/flash_procedure.md) を参照。
