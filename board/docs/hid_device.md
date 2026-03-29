# mydeck HID デバイス仕様

## 概要

mydeck は USB HID デバイスとしてホスト PC に認識される。
ボタンマトリクスの入力イベントをホストに送信し、ホストからの LED 制御コマンドを受信する。

## USB デバイス情報

| 項目 | 値 |
|------|-----|
| VID | `0x1209` (pid.codes) |
| PID | `0xDEC0` |
| USB | 1.10 (Low-Speed) |
| クラス | HID (Vendor Defined Usage Page `0xFF00`) |
| Manufacturer | UIAP |
| Product | mydeck |

## エンドポイント

| EP | 方向 | タイプ | パケットサイズ | ポーリング間隔 |
|----|------|--------|--------------|--------------|
| 0 | 双方向 | Control | 8 bytes | - |
| 1 | IN | Interrupt | 8 bytes | 10ms |

## レポート

すべてのレポートは 8 バイト固定長。

### Input Report (デバイス → ホスト)

ボタンの押下・リリース・長押しイベントを送信する。

| Byte | フィールド | 説明 |
|------|-----------|------|
| 0 | Report ID | `0x01` 固定 |
| 1 | buttonId | ボタン番号 (0 始まり、行優先の pin_index) |
| 2 | event | イベント種別 (下表参照) |
| 3-7 | reserved | 予約 (`0x00`) |

#### イベント種別

| 値 | 名前 | 説明 |
|----|------|------|
| `0x01` | Press | ボタン押下確定 (デバウンス完了後) |
| `0x02` | Release | ボタンリリース確定 (デバウンス完了後) |
| `0x03` | Hold | 長押し検出 |

### Output Report / Feature Report (ホスト → デバイス)

LED 制御などのコマンドを受信する。
UIAPduino では Feature Report (Report ID=`0x02`) として、Pro Micro では RawHID パケットとして受信する。

| Byte | フィールド | 説明 |
|------|-----------|------|
| 0 | Report ID / command | UIAPduino: `0x02` 固定 (Feature Report ID)、Pro Micro: コマンド種別 |
| 1 | command / param | UIAPduino: コマンド種別、Pro Micro: パラメータ |
| 2 | param / reserved | UIAPduino: パラメータ |
| 3-7 | reserved | 予約 (`0x00`) |

#### コマンド種別

| 値 | 名前 | 説明 |
|----|------|------|
| `0x01` | SetLed | TX LED の点灯/消灯。param=`0x00` で消灯、それ以外で点灯 |

## ボタン番号の割り当て

ボタンはマトリクス配線されており、buttonId は行優先 (row-major) で 0 から順に割り振られる。

```
buttonId = row * col_count + col
```

### Pro Micro (4 行 x 4 列 = 16 ボタン)

|       | Col0 | Col1 | Col2 | Col3 |
|-------|------|------|------|------|
| Row0  | 0    | 1    | 2    | 3    |
| Row1  | 4    | 5    | 6    | 7    |
| Row2  | 8    | 9    | 10   | 11   |
| Row3  | 12   | 13   | 14   | 15   |

### UIAPduino (3 行 x 5 列 = 15 ボタン)

|       | Col0 | Col1 | Col2 | Col3 | Col4 |
|-------|------|------|------|------|------|
| Row0  | 0    | 1    | 2    | 3    | 4    |
| Row1  | 5    | 6    | 7    | 8    | 9    |
| Row2  | 10   | 11   | 12   | 13   | 14   |

## ボタンイベントの挙動

### デバウンス

物理的なチャタリングを除去するため、状態変化から 50ms 間安定した場合にのみイベントを確定する。
チャタリング (ON→OFF→ON) が発生した場合、タイマーがリセットされ最後の変化から再度 50ms 待つ。

### Press / Release

- ボタンが OFF→ON に確定 → **Press** イベント発火
- ボタンが ON→OFF に確定 → **Release** イベント発火 (Hold が発火済みの場合は発火しない)

### Hold (長押し)

Press 確定から 500ms 間押し続けると **Hold** イベントが 1 回だけ発火する。
Hold 発火後にボタンを離しても Release イベントは発火しない。

### 複数ボタン同時押し

`update()` 1 回につき最大 1 イベントを返す。
pin_index の小さいボタンが優先的に処理される。

## LED の挙動

### TX LED (接続表示)

ホストからの SetLed コマンド、またはデバイス起動時に制御される。

- `led_service_set_connected(true)` → 点灯
- `led_service_set_connected(false)` → 消灯

### RX LED (通信表示)

ボタンイベント送信時に 80ms 間点灯する。
点灯中に再度イベントが発生した場合、タイマーがリセットされ再度 80ms 間点灯する。

### ボード別の LED 実装

| ボード | TX LED | RX LED |
|--------|--------|--------|
| Pro Micro | TXLED (オンボード) | RXLED (オンボード) |
| UIAPduino | 未実装 (no-op) | 未実装 (no-op) |

UIAPduino では LED3 (PC0) がモード表示 (実行モード=点灯、書き込み待機=消灯) に使用されており、TX/RX LED は未割り当て。
