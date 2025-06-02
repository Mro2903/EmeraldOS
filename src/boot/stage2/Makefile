BUILD_DIR?=build/
ASM?=nasm
ASMFLAGS?=-f obj
CC = /usr/bin/watcom/binl64/wcc
LD = /usr/bin/watcom/binl64/wlink
CFLAGS?=-4 -d3 -s -wx -ms -zl -zq

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/boot/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/boot/stage2/asm/%.obj, $(SOURCES_ASM))

.PHONY: clean always

$(BUILD_DIR)/boot/stage2/boot.bin: $(OBJECTS_C) $(OBJECTS_ASM) 
	$(LD) NAME $@ FILE \{ $^ \} OPTION MAP=$(BUILD_DIR)/boot/stage2/boot.map @linker.ld

$(BUILD_DIR)/boot/stage2/c/%.obj: %.c always
	${CC} ${CFLAGS} $< -fo=$@

$(BUILD_DIR)/boot/stage2/asm/%.obj: %.asm always
	$(ASM) $(ASMFLAGS) -o $@ $<

always:
	mkdir -p $(BUILD_DIR)/boot/stage2/c
	mkdir -p $(BUILD_DIR)/boot/stage2/asm

clean:
	rm -f $(BUILD_DIR)/stage2.bin