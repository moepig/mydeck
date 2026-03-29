# UIAPduino Pro Micro CH32V003 — mydeck
#
# ビルド:    make              (本番: デバッグ出力なし)
#            make debug        (デバッグ: SWIO printf 有効)
# 書き込み:  make flash        (本番ビルドを書き込み)
#            make flash-debug  (デバッグビルドを書き込み)
# モニタ:    make monitor      (minichlink でシリアルログ表示)
# クリーン:  make clean
# セットアップ (初回):  make setup
# 依存更新:  make update

TARGET     := main
TARGET_MCU := CH32V003

EXT_SRC  := ../../../ext_src
CH32FUN  ?= $(EXT_SRC)/ch32fun/ch32fun
RVUSB    ?= $(EXT_SRC)/rv003usb

CH32FUN_REPO := https://github.com/cnlohr/ch32fun.git
RVUSB_REPO   := https://github.com/cnlohr/rv003usb.git

# ---- ヘルプ -----------------------------------------------------

help:
	@echo "使い方:"
	@echo "  make setup       — 依存リポジトリの clone (未取得時) または pull + minichlink ビルド"
	@echo "  make update      — 依存リポジトリの pull + minichlink 再ビルド"
	@echo "  make             — ファームウェアをビルド (本番)"
	@echo "  make debug       — ファームウェアをビルド (デバッグ: SWIO printf 有効)"
	@echo "  make flash       — 本番ビルドをボードに書き込み"
	@echo "  make flash-debug — デバッグビルドをボードに書き込み"
	@echo "  make monitor     — シリアルログ表示 (minichlink -T)"
	@echo "  make clean       — ビルド成果物を削除"

# ---- セットアップ / 更新 ----------------------------------------

.PHONY: setup update help debug flash-debug

setup:
	@if [ ! -d "$(EXT_SRC)/ch32fun/.git" ]; then \
	    echo "[setup] Cloning ch32fun..."; \
	    git clone $(CH32FUN_REPO) $(EXT_SRC)/ch32fun; \
	else \
	    echo "[setup] ch32fun already cloned, pulling..."; \
	    git -C $(EXT_SRC)/ch32fun pull; \
	fi
	@if [ ! -d "$(EXT_SRC)/rv003usb/.git" ]; then \
	    echo "[setup] Cloning rv003usb..."; \
	    git clone $(RVUSB_REPO) $(EXT_SRC)/rv003usb; \
	else \
	    echo "[setup] rv003usb already cloned, pulling..."; \
	    git -C $(EXT_SRC)/rv003usb pull; \
	fi
	@echo "[setup] Building minichlink..."
	$(MAKE) -C $(EXT_SRC)/ch32fun/minichlink all

update:
	@echo "[update] Pulling ch32fun..."
	git -C $(EXT_SRC)/ch32fun pull
	@echo "[update] Pulling rv003usb..."
	git -C $(EXT_SRC)/rv003usb pull
	@echo "[update] Rebuilding minichlink..."
	$(MAKE) -C $(EXT_SRC)/ch32fun/minichlink all

# ---- ビルド -----------------------------------------------------

# エントリポイントは main.c — ch32fun.mk が $(TARGET).c として自動的にビルドする
# domain/ の .c は ADDITIONAL_C_FILES で追加

EXTRA_CFLAGS := \
    -I$(RVUSB)/rv003usb \
    -I$(RVUSB)/lib \
    -I. \
    -I../..

ADDITIONAL_C_FILES := \
    ../../domain/button_service.c \
    ../../domain/led_service.c \
    $(RVUSB)/rv003usb/rv003usb.c \
    $(RVUSB)/rv003usb/rv003usb.S

-include $(CH32FUN)/ch32fun.mk

# ---- デバッグビルド ---------------------------------------------

debug: clean
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS) -DMYDECK_DEBUG=1" build

# ---- 書き込み ---------------------------------------------------

UIAP_VIDPID := 0x1209b803

flash:
	$(MINICHLINK)/minichlink -c $(UIAP_VIDPID) -w $(TARGET).bin flash -b

flash-debug: clean
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS) -DMYDECK_DEBUG=1" build
	$(MINICHLINK)/minichlink -c $(UIAP_VIDPID) -w $(TARGET).bin flash -b

# monitor は ch32fun.mk が提供 (minichlink -T)

clean: cv_clean
