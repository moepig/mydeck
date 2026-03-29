#!/bin/bash
# rv003usb の PlatformIO ビルドスクリプトを生成する
#
# rv003usb.c と rv003usb.S は同名 .o を生成するため、.S を別ディレクトリでビルドする必要がある
# 公式の対処法: https://github.com/cnlohr/rv003usb/blob/master/.github/build_rv003usb.py
#
# このスクリプトは公式と同等の処理を行う Python スクリプトを board/ 直下に生成する
# 生成されたファイル (pio_build_rv003usb.py) は .gitignore 済み

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOARD_DIR="$(dirname "$SCRIPT_DIR")"
OUT="$BOARD_DIR/pio_build_rv003usb.py"

cat > "$OUT" << 'PYEOF'
import os.path
Import("env")
# rv003usb.c と rv003usb.S は同名 .o を生成するため、.S を別ディレクトリでビルドする
# see: https://github.com/cnlohr/rv003usb/blob/master/.github/build_rv003usb.py
env.BuildSources(
    os.path.join("$BUILD_DIR", "rv003usbASM"),
    os.path.join(env.subst("$PROJECT_DIR"), "ext_src", "rv003usb", "rv003usb"),
    src_filter=[
        "-<*>",
        "+<rv003usb.S>",
    ]
)
PYEOF

echo "Generated: $OUT"
