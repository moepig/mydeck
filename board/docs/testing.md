# ファームウェア ユニットテスト

ハードウェア不要。PC 上の `native` 環境で実行する。

## 前提条件

```bash
pip install platformio
```

## テスト実行

```bash
cd board

# 全テスト実行
pio test -e native

# 詳細出力付き
pio test -e native -v
```

ハードウェアへの書き込み手順は [flash_procedure.md](flash_procedure.md) を参照。
