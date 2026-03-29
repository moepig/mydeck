# UIAPduino — Makefile (ch32fun) で書き込み

## 前提条件

Ubuntu 24.04 LTS 推奨。詳細は
[UIAPduino ドキュメント](https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4) を参照。

```bash
# ビルドツールと依存ライブラリ
sudo apt install build-essential libnewlib-dev gcc-riscv64-unknown-elf \
                 libusb-1.0-0-dev libudev-dev gdb-multiarch
```

### 初回セットアップ

ch32fun / rv003usb の clone と minichlink のビルドを一括実行する。

```bash
cd board/src/boards/uiapduino
make setup
```

### udev ルール追加 (Linux)

```bash
sudo wget -O /etc/udev/rules.d/99-minichlink-uiap.rules \
  https://raw.githubusercontent.com/YuukiUmeta-UIAP/ch32fun/3bfa603f11d493710f2a811b5a2dfad905d9425c/minichlink/99-minichlink-uiap.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## ビルド

```bash
cd board/src/boards/uiapduino

# 本番ビルド
make build

# デバッグビルド (SWIO printf 有効)
make debug
```

## 書き込み

```bash
# ボードを書き込み待機モードにしてから実行
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

```bash
cd board/src/boards/uiapduino
make monitor
```

出力例:

```
mydeck: ready (rows=3 cols=5)
btn 1 ev 1
btn 1 ev 2
```
