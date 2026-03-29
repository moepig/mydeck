# board

マクロパッドのファームウェア。PlatformIO でビルド・テスト・書き込みを行う。

## ディレクトリ構成

```
board/
├── platformio.ini          PlatformIO プロジェクト設定（環境・ビルドフラグ）
├── Makefile                pio コマンドのショートカット (make build / test / flash)
├── src/                    ファームウェアソースコード
│   ├── mydeck.ino              Pro Micro エントリポイント (Arduino)
│   ├── domain/                 ドメイン層 — ボード非依存のビジネスロジック
│   │   ├── button_service.{h,cpp}   ボタンスキャン・デバウンス・イベント生成
│   │   ├── led_service.{h,cpp}      LED 点灯パターン制御
│   │   ├── hid_report.h             HID レポート構造体定義
│   │   ├── i_button_port.h          ボタン読み取りインターフェース
│   │   └── i_led_port.h             LED 制御インターフェース
│   ├── infrastructure/         インフラ層 — ハードウェア固有の IPort 実装
│   │   ├── arduino_button_port.h    Arduino (ATmega32U4) ボタン実装
│   │   ├── arduino_led_port.{h,cpp} Arduino LED 実装
│   │   ├── ch32_button_port.h       CH32V003 ボタン実装 (GPIOC 直接操作)
│   │   └── ch32_led_port.h          CH32V003 LED 実装
│   └── boards/                 ボード固有の設定・エントリポイント
│       ├── promicro/
│       │   └── board_config.h       Pro Micro ピン定義 (4x4 マトリクス)
│       └── uiapduino/
│           ├── board_config.h       UIAPduino ピン定義 (3x5 マトリクス)
│           ├── main.cpp             UIAPduino エントリポイント (ch32fun)
│           ├── usb_config.h         rv003usb USB ディスクリプタ定義
│           └── Makefile             ch32fun 単体ビルド用 (PlatformIO 不要)
├── test/                   ユニットテスト (Unity / PlatformIO native)
│   ├── test_button_service/
│   ├── test_led_service/
│   └── test_hid_report/
├── ext_src/                外部依存 (ch32fun, rv003usb) — .gitignore 済み
└── docs/                   設計・手順ドキュメント
    ├── flash_procedure.md      書き込み手順の目次・共通リファレンス
    ├── flash_pio_promicro.md   Pro Micro — PlatformIO で書き込み
    ├── flash_pio_uiapduino.md  UIAPduino — PlatformIO で書き込み
    ├── flash_make_uiapduino.md UIAPduino — Makefile (ch32fun) で書き込み
    ├── pin_assignment.md       ピンアサイン・マトリクス配線図
    └── testing.md              テスト実行手順
```

## 対応ボード

| ボード | MCU | マトリクス | PlatformIO env |
|--------|-----|-----------|----------------|
| SparkFun Pro Micro | ATmega32U4 | 4x4 (16 ボタン) | `sparkfun_promicro16` |
| UIAPduino Pro Micro | CH32V003 (RISC-V) | 3x5 (15 ボタン) | `uiapduino` |

## クイックスタート

```bash
cd board

# ユニットテスト
make test

# Pro Micro ビルド・書き込み
make build
make flash

# UIAPduino ビルド・書き込み
make build ENV=uiapduino
make flash ENV=uiapduino
```

詳細は [docs/flash_procedure.md](docs/flash_procedure.md) を参照。
