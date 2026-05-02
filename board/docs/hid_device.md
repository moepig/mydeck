# mydeck USB HID デバイス仕様

## 概要

mydeck は USB HID デバイスとしてホスト PC に認識される。
ボタンマトリクスの入力イベントをホストに送信し、ホストからの LED 制御コマンドを受信する。

---

## USB デバイス情報

| 項目 | UIAPduino | Pro Micro |
|------|-----------|-----------|
| VID | `0x1209` (pid.codes) | `0x1B4F` (SparkFun) |
| PID | `0xDEC0` | `0x9206` (RawHID) |
| USB バージョン | 1.10 (Low-Speed) | 1.10 (Full-Speed) |
| クラス | HID (Vendor Usage Page `0xFF00`) | HID RawHID |
| Manufacturer | `UIAP` | `SparkFun` |
| Product | `mydeck` | `mydeck` |

---

## エンドポイント

### UIAPduino

| EP | 方向 | タイプ | パケットサイズ | ポーリング間隔 |
|----|------|--------|--------------|--------------|
| 0 | 双方向 | Control | 8 bytes | - |
| 1 | IN | Interrupt | 8 bytes | 10ms |

ホスト → デバイスの通信は Control 転送の HID Feature Report (SET_REPORT / GET_REPORT) を使用する。

### Pro Micro

| EP | 方向 | タイプ | パケットサイズ | ポーリング間隔 |
|----|------|--------|--------------|--------------|
| 0 | 双方向 | Control | 8 bytes | - |
| 3 | IN/OUT | Interrupt (RawHID) | 64 bytes | 1ms |

---

## HID レポートレイアウト

すべてのレポートは先頭 3 バイトが有効フィールド、残りは予約 (`0x00`)。

### Input Report — デバイス → ホスト

ボタンの押下・リリース・長押しイベントを通知する。

| Byte | フィールド | 値 |
|------|-----------|-----|
| 0 | Report ID | `0x01` 固定 |
| 1 | buttonId | ボタン番号 (`0x00`〜最大ボタン数−1) |
| 2 | event | イベント種別 (下表参照) |
| 3〜 | reserved | `0x00` |

- UIAPduino: 8 バイト固定 (Interrupt IN EP1)
- Pro Micro: 64 バイト固定 (RawHID)、有効データは先頭 3 バイト

#### event 値

| 値 | 名前 | 説明 |
|----|------|------|
| `0x01` | Press | ボタン押下確定 (デバウンス完了後) |
| `0x02` | Release | ボタンリリース確定 (デバウンス完了後) |
| `0x03` | Hold | 長押し検出 (Press 確定から 500ms 後に 1 回だけ) |

### Output Report — ホスト → デバイス

LED 制御などのコマンドを送信する。

| Byte | フィールド | 値 |
|------|-----------|-----|
| 0 | Report ID | `0x02` 固定 |
| 1 | command | コマンド種別 (下表参照) |
| 2 | param | コマンドのパラメータ |
| 3〜 | reserved | `0x00` |

- UIAPduino: HID Feature Report (SET_REPORT, Report ID=`0x02`)
- Pro Micro: RawHID 64 バイトパケット、先頭 3 バイトが有効

#### command 値

| 値 | 名前 | 説明 |
|----|------|------|
| `0x01` | SetLed | TX LED 制御。param=`0x00` で消灯、`0x01` で点灯 |

---

## ボタン番号の割り当て

ボタンはマトリクス配線されており、`buttonId` は行優先 (row-major) で 0 から順に割り振られる。

```
buttonId = row * col_count + col
```

### Pro Micro / UIAPduino 共通 (2 行 × 6 列 = 12 ボタン)

|       | Col0 | Col1 | Col2 | Col3 | Col4 | Col5 |
|-------|------|------|------|------|------|------|
| Row0  | 0    | 1    | 2    | 3    | 4    | 5    |
| Row1  | 6    | 7    | 8    | 9    | 10   | 11   |

---

## ボタンイベントの挙動

### デバウンス

物理的なチャタリングを除去するため、状態変化から 50ms 間安定した場合にのみイベントを確定する。
チャタリング (ON→OFF→ON) が発生した場合、タイマーがリセットされ最後の変化から再度 50ms 待つ。

### Press / Release

- ボタンが OFF→ON に確定 → **Press** イベント送信
- ボタンが ON→OFF に確定 → **Release** イベント送信
  - ただし、同ボタンで Hold が発火済みの場合は Release を送信しない

### Hold (長押し)

- Press 確定から 500ms 間押し続けると **Hold** イベントを 1 回だけ送信する
- Hold 発火後にボタンを離しても Release イベントは送信しない

### 複数ボタン同時押し

`button_service_update()` 1 回につき最大 1 イベントを返す。
`buttonId` の小さいボタンが優先的に処理される。

---

## LED の挙動

### TX LED (接続表示)

ホストからの SetLed コマンドで制御される。デバイス起動時は点灯状態で初期化される。

| param | 状態 |
|-------|------|
| `0x00` | 消灯 |
| `0x01` (または `0x00` 以外) | 点灯 |

### RX LED (通信表示、Pro Micro のみ)

ボタンイベント送信時に 80ms 間点灯する。
点灯中に再度イベントが発生した場合、タイマーがリセットされ再度 80ms 間点灯する。

### ボード別の LED 実装

| ボード | TX LED | RX LED |
|--------|--------|--------|
| Pro Micro | TXLED (オンボード) | RXLED (オンボード) |
| UIAPduino | 未実装 | 未実装 |

UIAPduino では PC0 (LED3) がモード表示専用 (実行モード=点灯、書き込み待機=消灯) に使用されており、TX/RX LED は未割り当て。

---

## HID レポートディスクリプタ (UIAPduino)

```
Usage Page (Vendor Defined 0xFF00)
Usage (0x01)
Collection (Application)
  Report ID (1)          ← Input Report
  Usage (0x01)
  Logical Min/Max (0〜255)
  Report Size (8)
  Report Count (7)       ← 7 bytes (Report ID 除く)
  Input (Data, Variable, Absolute)

  Report ID (2)          ← Feature Report
  Usage (0x02)
  Logical Min/Max (0〜255)
  Report Size (8)
  Report Count (7)
  Feature (Data, Variable, Absolute)
End Collection
```
