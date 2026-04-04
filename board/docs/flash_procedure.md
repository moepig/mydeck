# ボードへの書き込み手順

| ボード | 書き込み方法 | 手順 |
|--------|------------|------|
| Pro Micro (ATmega32U4) | PlatformIO | [flash_pio_promicro.md](flash_pio_promicro.md) |
| UIAPduino (CH32V003) | PlatformIO | [flash_pio_uiapduino.md](flash_pio_uiapduino.md) |
| UIAPduino (CH32V003) | Makefile (ch32fun) | [flash_make_uiapduino.md](flash_make_uiapduino.md) |

## ユニットテスト（PC 上で実行）

```powershell
cd board
pio test -e native
```

---

## UIAPduino 共通リファレンス

### 書き込み待機モードへの切り替え

ボードを書き込み待機モードにしてから `make flash` または `pio run --target upload` を実行する。

- **初回 / シームレススイッチ無効時**: リセットボタンを押しながら USB 接続 → すぐにボタンを離す
- **シームレススイッチ有効時**: リセットボタンを押すだけで切り替わる（USB の抜き差し不要）

### LED3 によるモード判別

LED3 (PC0) の点灯状態で現在のモードを確認できる。

| LED3 の状態 | モード | 説明 |
|------------|--------|------|
| 点灯 | 実行モード | ファームウェアが動作中 |
| 消灯 | 書き込み待機モード | ブートローダが動作中。書き込み可能 |

### シームレススイッチ

`main.c` の `seamless_switch_init()` で実装済み。
リセットボタンを押すと書き込み待機モード↔実行モードが切り替わる。
USB ケーブルの抜き差しは不要。
LED3 が消灯したら書き込み待機モードに入ったことを示す。

### 注意事項

- 複数の UIAPduino が同時に書き込み待機状態だと識別できず失敗する
- USB 給電専用ケーブル（データ線なし）では書き込みできない
- 通信が不安定な場合は別の USB ポートやハブを試す
