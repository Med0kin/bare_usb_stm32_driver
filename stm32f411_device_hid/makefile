# Toolchain and build parameters
CC = arm-none-eabi-gcc
BUILD_DIR = build
MACH = cortex-m4
DEBUG = 1
SPECS = rdimon.specs

# Source files
C_SOURCES = \
core/src/stm32_startup.c \
core/src/main.c \
core/src/isr.c \
core/src/qassert.c \
core/src/clock.c \
usb/src/usb.c \
usb/src/usb_isr.c

# Include directories
C_INCLUDES = \
-Icore/inc \
-Icmsis/cmsis-device-f4/Include \
-Icmsis/cmsis-core/Include \
-Iusb/inc

# Defines
C_DEFINES = \
-DSTM32F411xE

# Flags
CFLAGS += -c -mcpu=$(MACH) $(C_DEFINES) $(C_INCLUDES) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=$(SPECS) -T stm32_ls.ld -Wl,-Map=$(BUILD_DIR)/final.map

# Bulid target
TARGET = final
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.PHONY: all clean openocd flash

all: $(TARGET)

clean:
	rm -f $(BUILD_DIR)/*

openocd:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/final.elf verify reset exit"
