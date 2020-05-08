PROJECT = awesomesauce
BUILD_DIR = bin

#SHARED_DIR = ../my-common-code
CFILES = src/hwinit.c src/main.c src/spi_func.c src/usart.c src/usb_setup.c  src/snes_min_ctrl.c
#CFILES += api.c
#AFILES += api-asm.S
INCLUDES += -I src/

#tmp...
TGT_CFLAGS+=-Wno-missing-prototypes -Wno-unused-variable -Wno-missing-prototypes -Wno-unused-parameter -Wno-missing-prototypes

# TODO - you will need to edit these two lines!
DEVICE=stm32f103c8
#OOCD_FILE = board/stm32f4discovery.cfg

#VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk


stflash: $(PROJECT).elf $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x8000000