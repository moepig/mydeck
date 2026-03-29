# ボードへの書き込み手順

対応ボード: Pro Micro (`sparkfun_promicro16`) / UIAPduino (`uiapduino`)

## 前提条件

- [PlatformIO IDE](https://platformio.org/) または PlatformIO Core (CLI) がインストール済み
- ボードが USB で接続済み

## 1. PlatformIO Core のインストール（CLI）

```bash
pip install platformio
```

## 2. ビルド

```bash
cd src/mydeck/board
# Pro Micro
pio run -e sparkfun_promicro16
# UIAPduino
pio run -e uiapduino
```

## 3. 書き込み

```bash
# Pro Micro
pio run -e sparkfun_promicro16 --target upload
# UIAPduino
pio run -e uiapduino --target upload
```

### 書き込みに失敗する場合（Pro Micro のブートローダ起動）

Pro Micro はリセットボタンがないため、以下の手順でブートローダを起動する。

1. `pio run --target upload` を実行してアップロード待ち状態にする
2. RST ピンを素早く 2 回 GND に触れる（ダブルタップリセット）
3. 8 秒以内に書き込みが開始される

```bash
# シリアルポートを明示する場合
pio run -e sparkfun_promicro16 --target upload --upload-port COM3
# Linux/Mac の場合
pio run -e sparkfun_promicro16 --target upload --upload-port /dev/ttyACM0
```

## 4. シリアルモニタ（デバッグ用）

```bash
pio device monitor --baud 115200
```

## 5. ユニットテスト（PC 上で実行）

```bash
pio test -e native
```

## 6. デバイス認識確認（Windows）

デバイスマネージャーで以下を確認：

- `ヒューマン インターフェイス デバイス` → `MyDeck HID Device`
- VID: `0x1B4F`, PID: `0x9206`

または PowerShell で確認：

```powershell
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*MyDeck*" }
```
