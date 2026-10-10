BUILD := build
TARGET := $(BUILD)/noos

ifeq ($(LLVM),1)
CC := clang
OBJCOPY := llvm-objcopy
OBJDUMP := llvm-objdump
NM := llvm-nm
SIZE := size
ARCHFLAGS := --target=arm-none-eabi -march=armv7-a -marm
LINKFLAGS := -fuse-ld=lld
else
CROSS ?= arm-none-eabi-
CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
NM := $(CROSS)nm
SIZE := $(CROSS)size
ARCHFLAGS := -march=armv7-a -marm
LINKFLAGS :=
endif

PYTHON ?= python3
BOOT_CMDLINE ?= lk2nd.pass-simplefb=xrgb8888
UARTDM_HANDOFF ?= 0
CPPFLAGS := -Iinclude -DNOOS_UARTDM_HANDOFF=$(UARTDM_HANDOFF)
CFLAGS := $(ARCHFLAGS) -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fdata-sections -ffunction-sections -Wall -Wextra -Werror -Os
ASFLAGS := $(ARCHFLAGS) -ffreestanding -x assembler-with-cpp
LDFLAGS := $(ARCHFLAGS) $(LINKFLAGS) -nostdlib -Wl,--gc-sections -Wl,-Map,$(TARGET).map -T linker.ld

OBJECTS := \
	$(BUILD)/arch/arm/start.o \
	$(BUILD)/src/kernel.o \
	$(BUILD)/src/boot.o \
	$(BUILD)/src/console.o \
	$(BUILD)/src/cpu.o \
	$(BUILD)/src/diagnostic.o \
	$(BUILD)/src/exception.o \
	$(BUILD)/src/fdt.o \
	$(BUILD)/src/framebuffer.o \
	$(BUILD)/src/heap.o \
	$(BUILD)/src/memory.o \
	$(BUILD)/src/mmu.o \
	$(BUILD)/src/panic.o \
	$(BUILD)/src/pmm.o \
	$(BUILD)/src/selftest.o \
	$(BUILD)/src/status.o \
	$(BUILD)/drivers/uart/uartdm.o \
	$(BUILD)/platform/rm915/platform.o

.PHONY: all clean disasm size check-toolchain bootimg verify manifest

all: check-toolchain $(TARGET).elf $(TARGET).bin $(TARGET)-boot.img verify manifest size

check-toolchain:
	@command -v $(CC) >/dev/null || { echo "error: $(CC) not found"; exit 1; }
	@command -v $(OBJCOPY) >/dev/null || { echo "error: $(OBJCOPY) not found"; exit 1; }
	@command -v $(NM) >/dev/null || { echo "error: $(NM) not found"; exit 1; }
	@command -v $(PYTHON) >/dev/null || { echo "error: $(PYTHON) not found"; exit 1; }

$(TARGET).elf: $(OBJECTS) linker.ld
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET)-boot.img: $(TARGET).bin tools/mkbootimg.py
	$(PYTHON) tools/mkbootimg.py $< $@ --cmdline "$(BOOT_CMDLINE)"

bootimg: $(TARGET)-boot.img

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/arch/arm/%.o: arch/arm/%.S
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

verify: $(TARGET)-boot.img tools/verify_bootimg.py
	$(PYTHON) tools/verify_bootimg.py $(TARGET)-boot.img --expect-cmdline "$(BOOT_CMDLINE)"

manifest: $(TARGET).elf $(TARGET).bin $(TARGET)-boot.img tools/build_manifest.py
	$(PYTHON) tools/build_manifest.py --elf $(TARGET).elf --bin $(TARGET).bin --boot $(TARGET)-boot.img --output $(TARGET)-manifest.json --nm $(NM) --boot-cmdline "$(BOOT_CMDLINE)"

disasm: $(TARGET).elf
	$(OBJDUMP) -D $< > $(TARGET).lst
	@echo "wrote $(TARGET).lst"

size: $(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf $(BUILD)
