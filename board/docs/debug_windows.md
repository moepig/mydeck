# Windows でのデバッグ手順 — UIAPduino

## 1. デバイス認識の確認

デバイスマネージャー (`devmgmt.msc`) → 「ヒューマン インターフェイス デバイス」に
**mydeck** が表示されれば USB 接続・ファームウェア動作ともに正常。

PowerShell でも確認できる:

```powershell
Get-PnpDevice | Where-Object { $_.InstanceId -like "*1209*DEC0*" -or $_.FriendlyName -like "*mydeck*" }
```

正常時の期待値:

| 項目 | 値 |
|------|----|
| VID  | `0x1209` |
| PID  | `0xDEC0` |
| 名前 | mydeck |
| ステータス | OK |

---

## 2. よくあるエラーと対処

### コード 43「USB デバイス記述子の要求が失敗しました」

ファームウェアが USB 列挙を完了できていない状態。以下を順番に確認する。

#### 2-1. LED3 でモードを確認

[LED3 によるモード判別](flash_procedure.md#led3-によるモード判別) を参照。

| LED3 | 状態 | 意味 |
|------|------|------|
| 点灯 | 実行モード | ファームウェア動作中 |
| 消灯 | 書き込み待機モード | ブートローダ動作中（別の VID/PID で列挙） |

書き込み待機モードのままだとコード 43 が出ることがある。
リセットボタンを押して実行モードに切り替えてから再接続する。

#### 2-2. ケーブルを確認

充電専用ケーブル（データ線なし）では列挙できない。
別のデータ通信対応ケーブルに替えて試す。

#### 2-3. USB ポートを替える

USB ハブ経由の場合はパソコン本体の USB ポートに直接接続する。
USB 3.x ポートで不安定な場合は USB 2.0 ポートを試す。

#### 2-4. ファームウェアを再書き込みする

書き込みが不完全だと USB が起動しないことがある。

```powershell
cd board
pio run -e uiapduino --target upload
```

書き込み待機モードへの切り替え方は [flash_procedure.md](flash_procedure.md#書き込み待機モードへの切り替え) を参照。

---

## 3. HID 動作確認（ボタン入力のモニタ）

デバイスが正常認識されたら、ボタン押下時の HID レポートを確認できる。

### hidapitester のインストール

GitHub Releases からバイナリをダウンロードする:

```
https://github.com/todbot/hidapitester/releases
```

`hidapitester-windows.zip` を展開し、`hidapitester.exe` を PATH の通ったフォルダ（例: `C:\Windows\System32` や任意のディレクトリ）に置く。

### デバイスの列挙

```powershell
hidapitester --list
```

`mydeck` または VID/PID `1209/DEC0` のエントリが表示されることを確認する。

### ボタン入力のモニタ

```powershell
hidapitester --vidpid 1209/DEC0 --usage 1 --usagePage FF00 --open --read-input-forever
```

ボタンを押すと 7 バイトのレポートが表示される:

```
# 例: ボタン 3 を押した場合
01 03 01 00 00 00 00 00
^  ^  ^
|  |  └ event (01=PRESS, 02=RELEASE, 03=HOLD)
|  └─── buttonId (0-14, 0 始まり = pin_index)
└─────── Report ID (常に 0x01)
```

---

## 4. Wireshark で USB パケットをキャプチャする

USBPcap（Wireshark に同梱）を使うと USB プロトコルレベルでデバッグできる。

### キャプチャ時に対象デバイスを絞る

Wireshark 起動 → インターフェース一覧の USBPcap を選択 → 歯車アイコンで設定を開く。
一覧に表示されているハブ・ポートのうち、mydeck が接続されているものだけにチェックを入れてキャプチャ開始する。
（デバイスが接続済みでないと一覧に出ないため、先に接続しておく）

### 表示フィルタで絞る

キャプチャ後に Filter バーへ入力する。

**VID/PID で絞る**（enumeration パケットが含まれている場合のみ有効）

```
usb.idVendor == 0x1209 && usb.idProduct == 0xdec0
```

**デバイスアドレスで絞る**（enumeration 後の通常トラフィックに有効）

```
usb.device_address == 3
```

デバイスアドレスは `usb.device_address` カラムか、接続直後の `SET_ADDRESS` パケットで確認できる。

**バス番号も合わせて絞る**（複数 USBPcap インターフェースがある場合）

```
usb.bus_id == 1 && usb.device_address == 3
```

### 手順まとめ

1. mydeck を接続した状態で Wireshark を起動し、対象の USBPcap インターフェースを選択
2. `usb.idVendor == 0x1209` で絞り、`SET_ADDRESS` パケットからデバイスアドレスを確認
3. `usb.device_address == <確認した番号>` に切り替えて解析する

---

## 5. SWIO デバッグ printf（WCH-Link が必要）

`uiapduino_debug` ビルドの `printf` 出力は SWIO (PD1) 経由。
WCH-Link E を PD1 に接続すれば minichlink で読める。

```powershell
# WCH-Link を接続した状態で実行
C:\Users\uncho\.platformio\packages\tool-minichlink\minichlink.exe -T
```

WCH-Link を持っていない場合はこの方法は使えない。
代わりに上記 **3. HID 動作確認** でボタン動作を確認する。
