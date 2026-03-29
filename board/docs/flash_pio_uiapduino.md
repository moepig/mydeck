# UIAPduino — PlatformIO で書き込み

## 前提条件

- [PlatformIO Core (CLI)](https://platformio.org/) がインストール済み
- ch32fun / rv003usb が取得済み（未取得の場合は先に初回セットアップを実行）
- minichlink がビルド済み（PlatformIO が `upload_protocol = minichlink` で使用）

```bash
pip install platformio
```

### 初回セットアップ（依存ライブラリ取得）

```bash
# ビルドツール
sudo apt install build-essential libnewlib-dev gcc-riscv64-unknown-elf \
                 libusb-1.0-0-dev libudev-dev

# ch32fun / rv003usb clone + minichlink ビルド
cd board/src/boards/uiapduino
make setup
```

### udev ルール追加 (Linux)

```bash
sudo wget -O /etc/udev/rules.d/99-minichlink-uiap.rules \
  https://raw.githubusercontent.com/YuukiUmeta-UIAP/ch32fun/3bfa603f11d493710f2a811b5a2dfad905d9425c/minichlink/99-minichlink-uiap.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### ビルドスクリプト生成

rv003usb の PlatformIO ビルドに必要なスクリプトを生成する（初回および `ext_src/` 更新後）。

```bash
cd board
scripts/fetch_build_rv003usb.sh
```

## ビルド

```bash
cd board

# 本番ビルド
pio run -e uiapduino

# デバッグビルド (SWIO printf 有効)
pio run -e uiapduino_debug
```

## 書き込み

```bash
# ボードを書き込み待機モードにしてから実行
cd board

# 本番ビルドを書き込み
pio run -e uiapduino --target upload

# デバッグビルドを書き込み
pio run -e uiapduino_debug --target upload
```

書き込み待機モードへの切り替え方法は [共通リファレンス](flash_procedure.md#書き込み待機モードへの切り替え) を参照。

## シリアルログ (デバッグビルド)

デバッグビルドでは SWIO (PD1) 経由で printf 出力が有効になる。
minichlink を接続したまま以下のいずれかでログを表示する。

```bash
# PlatformIO (platform-ch32v 提供のターゲット)
cd board
pio run -e uiapduino_debug -t sdi_printf_monitor

# Makefile 経由
cd board/src/boards/uiapduino
make monitor
```

## デバイス認識確認

- VID: `0x1209`, PID: `0xDEC0`（`usb_config.h` で変更可）

```bash
# Linux
lsusb | grep "1209:dec0"
```
