# UIAPduino — PlatformIO で書き込み

## 前提条件

- [PlatformIO Core (CLI)](https://platformio.org/) がインストール済み
- ch32fun / rv003usb が取得済み（未取得の場合は先に初回セットアップを実行）
- minichlink がビルド済み（PlatformIO が `upload_protocol = minichlink` で使用）

```powershell
pip install platformio
```

### 初回セットアップ（依存ライブラリ取得）

Windows では [MSYS2](https://www.msys2.org/) を使用してビルドツールをインストールする。

```powershell
# MSYS2 インストール後、MSYS2 UCRT64 ターミナルで実行
pacman -S mingw-w64-ucrt-x86_64-gcc make mingw-w64-ucrt-x86_64-libusb
```

RISC-V ツールチェーンは [xPack GNU RISC-V Embedded GCC](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) からダウンロードし、PATH に追加する。

```powershell
# ch32fun / rv003usb clone + minichlink ビルド (MSYS2 UCRT64 ターミナルで実行)
cd board/src/boards/uiapduino
make setup
```

### WinUSB ドライバのインストール

minichlink が USB デバイスにアクセスするために [Zadig](https://zadig.akeo.ie/) で WinUSB ドライバをインストールする。

1. UIAPduino を書き込み待機モードで USB 接続する
2. Zadig を起動し、Options → List All Devices にチェック
3. デバイス一覧から UIAPduino (WCH-Link) を選択
4. ドライバを **WinUSB** に設定し、Install Driver をクリック

### ビルドスクリプト生成

rv003usb の PlatformIO ビルドに必要なスクリプトを生成する（初回および `ext_src/` 更新後）。

```powershell
cd board
python scripts/fetch_build_rv003usb.py
```

## ビルド

```powershell
cd board

# 本番ビルド
pio run -e uiapduino

# デバッグビルド (SWIO printf 有効)
pio run -e uiapduino_debug
```

## 書き込み

```powershell
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

```powershell
# PlatformIO (platform-ch32v 提供のターゲット)
cd board
pio run -e uiapduino_debug -t sdi_printf_monitor

# MSYS2 ターミナルから Makefile 経由
cd board/src/boards/uiapduino
make monitor
```

## デバイス認識確認

- VID: `0x1209`, PID: `0xDEC0`（`usb_config.h` で変更可）

```powershell
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*mydeck*" }
```
