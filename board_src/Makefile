.PHONY: all build test flash clean

ENV ?= sparkfun_promicro16

all: build

## ファームウェアをビルドする
build:
	pio run -e $(ENV)

## ネイティブユニットテストを実行する
test:
	pio test -e native

## ボードにファームウェアを書き込む
flash:
	pio run -e $(ENV) --target upload

clean:
	pio run --target clean
