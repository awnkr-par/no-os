BUILD := build
TARGET := $(BUILD)/noos

ifeq ($(LLVM),1)
CC := clang
OBJCOPY := llvm-objcopy
OBJDUMP := llvm-objdump
SIZE := size
ARCHFLAGS := --target=arm-none-eabi -march=armv7-a -marm
LINKFLAGS := -fuse-ld=lld
else
CROSS ?= arm-none-eabi-
CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE := $(CROSS)size
ARCHFLAGS := -march=armv7-a -marm
LINKFLAGS :=
endif

PYTHON ?= python3
CPPFLAGS := -Iinclude
CFLAGS := $(ARCHFLAGS) -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fdata-sections -ffunction-sections -Wall -Wextra -Werror -Os
ASFLAGS := $(ARCHFLAGS) -ffreestanding -x assembler-with-cpp
LDFLAGS := $(ARCHFLAGS) $(LINKFLAGS) -nostdlib -Wl,--gc-sections -Wl,-Map,$(TARGET).map -T linker.ld

OBJECTS := \
	$(BUILD)/arch/arm/start.o \
	$(BUILD)/src/kernel.o \
	$(BUILD)/src/boot.o \
	$(BUILD)/src/console.o \
	$(BUILD)/src/memory.o \
	$(BUILD)/src/panic.o \
	$(BUILD)/src/pmm.o \
	$(BUILD)/platform/rm915/platform.o

.PHONY: all clean disasm size check-toolchain bootimg

all: check-toolchain $(TARGET).elf $(TARGET).bin $(TARGET)-boot.img size

check-toolchain:
	@command -v $(CC) >/dev/null || { echo "error: $(CC) not found"; exit 1; }
	@command -v $(OBJCOPY) >/dev/null || { echo "error: $(OBJCOPY) not found"; exit 1; }
	@command -v $(PYTHON) >/dev/null || { echo "error: $(PYTHON) not found"; exit 1; }

$(TARGET).elf: $(OBJECTS) linker.ld
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET)-boot.img: $(TARGET).bin tools/mkbootimg.py
	$(PYTHON) tools/mkbootimg.py $< $@

bootimg: $(TARGET)-boot.img

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/arch/arm/%.o: arch/arm/%.S
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

disasm: $(TARGET).elf
	$(OBJDUMP) -D $< > $(TARGET).lst
	@echo "wrote $(TARGET).lst"

size: $(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf $(BUILD)
