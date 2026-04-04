# UIAPduino — Makefile (ch32fun) で書き込み

## 前提条件

Windows では [MSYS2](https://www.msys2.org/) を使用する。詳細は
[UIAPduino ドキュメント](https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4) を参照。

```
# MSYS2 UCRT64 ターミナルで実行
pacman -S mingw-w64-ucrt-x86_64-gcc make mingw-w64-ucrt-x86_64-libusb
```

RISC-V ツールチェーンは [xPack GNU RISC-V Embedded GCC](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) からダウンロードし、PATH に追加する。

### 初回セットアップ

ch32fun / rv003usb の clone と minichlink のビルドを一括実行する。

```
# MSYS2 UCRT64 ターミナルで実行
cd board/src/boards/uiapduino
make setup
```

### WinUSB ドライバのインストール

minichlink が USB デバイスにアクセスするために [Zadig](https://zadig.akeo.ie/) で WinUSB ドライバをインストールする。

1. UIAPduino を書き込み待機モードで USB 接続する
2. Zadig を起動し、Options → List All Devices にチェック
3. デバイス一覧から UIAPduino (WCH-Link) を選択
4. ドライバを **WinUSB** に設定し、Install Driver をクリック

## ビルド

```
# MSYS2 UCRT64 ターミナルで実行
cd board/src/boards/uiapduino

# 本番ビルド
make build

# デバッグビルド (SWIO printf 有効)
make debug
```

## 書き込み

```
# ボードを書き込み待機モードにしてから実行 (MSYS2 UCRT64 ターミナル)
cd board/src/boards/uiapduino

# 本番ビルドを書き込み
make flash

# デバッグビルドを書き込み
make flash-debug
```

`Image written.` が表示されたら完了。ボードが自動的に実行モードに移行する。

書き込み待機モードへの切り替え方法は [共通リファレンス](flash_procedure.md#書き込み待機モードへの切り替え) を参照。

## シリアルログ (デバッグビルド)

デバッグビルドでは SWIO (PD1) 経由で printf 出力が有効になる。
minichlink を接続したまま以下のコマンドでログを表示する。

```
# MSYS2 UCRT64 ターミナルで実行
cd board/src/boards/uiapduino
make monitor
```

出力例:

```
mydeck: ready (rows=3 cols=5)
btn 1 ev 1
btn 1 ev 2
```
