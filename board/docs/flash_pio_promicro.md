# Pro Micro — PlatformIO で書き込み

## 前提条件

- [PlatformIO Core (CLI)](https://platformio.org/) がインストール済み
- ボードが USB で接続済み

```powershell
pip install platformio
```

## ビルド

```powershell
cd board
pio run -e sparkfun_promicro16
```

## 書き込み

```powershell
cd board
pio run -e sparkfun_promicro16 --target upload
```

### ブートローダ起動（書き込みに失敗する場合）

Pro Micro はリセットボタンがないため RST ピンで操作する。

1. `pio run -e sparkfun_promicro16 --target upload` を実行してアップロード待ちにする
2. RST ピンを素早く 2 回 GND に触れる（ダブルタップリセット）
3. 8 秒以内に書き込みが開始される

シリアルポートを明示する必要がある場合は、まず接続先の COM ポートを特定する。

```powershell
# 接続中のシリアルポート一覧を表示
Get-PnpDevice -Class Ports -Status OK | Select-Object FriendlyName
```

出力例:

```
FriendlyName
------------
USB Serial Device (COM3)
```

表示された COM ポートを `--upload-port` に指定する。

```powershell
pio run -e sparkfun_promicro16 --target upload --upload-port COM3
```

## デバイス認識確認

- VID: `0x1B4F`, PID: `0x9206`

```powershell
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*mydeck*" }
```
