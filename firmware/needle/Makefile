all : flash

TARGET:=needle
TARGET_MCU:=CH572D

ADDITIONAL_C_FILES:=../ch32fun/extralibs/fsusb.c

include ../ch32fun/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean

USB_SERIAL ?= /dev/cu.usbmodem*

autoflash : $(TARGET).bin $(MINICHLINK)/minichlink
	@echo "Waiting for $(USB_SERIAL) to send BOOT..."
	@for i in $$(seq 300); do for port in $(USB_SERIAL); do if [ -e "$$port" ]; then printf BOOT > "$$port" && exit 0; fi; done; sleep 0.1; done; echo "No USB serial port found"; exit 1
	@for i in $$(seq 20); do $(FLASH_COMMAND) && exit 0; sleep 0.5; done; exit 1
