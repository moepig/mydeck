# Pro Micro — PlatformIO で書き込み

## 前提条件

- [PlatformIO Core (CLI)](https://platformio.org/) がインストール済み
- ボードが USB で接続済み

```bash
pip install platformio
```

## ビルド

```bash
cd board
pio run -e sparkfun_promicro16
```

## 書き込み

```bash
cd board
pio run -e sparkfun_promicro16 --target upload
```

### ブートローダ起動（書き込みに失敗する場合）

Pro Micro はリセットボタンがないため RST ピンで操作する。

1. `pio run -e sparkfun_promicro16 --target upload` を実行してアップロード待ちにする
2. RST ピンを素早く 2 回 GND に触れる（ダブルタップリセット）
3. 8 秒以内に書き込みが開始される

```bash
# シリアルポートを明示する場合 (Linux)
pio run -e sparkfun_promicro16 --target upload --upload-port /dev/ttyACM0
```

## デバイス認識確認

- VID: `0x1B4F`, PID: `0x9206`

```powershell
# Windows
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*mydeck*" }
```
