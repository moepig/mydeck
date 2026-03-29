# Pro Micro (ATmega32U4) ボード固有ファイル

## エントリポイント

Pro Micro のエントリポイントは `src/mydeck.cpp`。

### `.ino` ではなく `.cpp` を使う理由

PlatformIO は `src_dir = .` の構成で `.ino` ファイルを `src_dir` のルートからのみ自動検出する。
本プロジェクトは UIAPduino と `src_dir` を共有するため、エントリポイントは `src/mydeck.cpp` に配置しており、`build_src_filter` 経由でコンパイル対象に含める必要がある。
`.ino` は `build_src_filter` では拾われないが、`.cpp` なら通常のソースファイルとして処理される。

`.ino` → `.cpp` の差分は `#include <Arduino.h>` の明示的な追加のみ。
