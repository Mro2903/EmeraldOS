SRC_DIR=src
BUILD_DIR=build


C_SOURCES = $(wildcard ${SRC_DIR}/kernel/*.c ${SRC_DIR}/drivers/*.c)
HEADERS = $(wildcard ${SRC_DIR}/kernel/*.h ${SRC_DIR}/drivers/*.h)
# Nice syntax for file extension replacement
OBJ = $(patsubst ${SRC_DIR}/%,${BUILD_DIR}/%,$(C_SOURCES:.c=.o))

# Change this if your cross-compiler is somewhere else
CC = /usr/bin/watcom/binl64/wcc
LD = /usr/bin/watcom/binl64/wlink
CFLAGS=-4 -d3 -s -wx -ms -zl -zq

.PHONY: clean run debug bootloader

# First rule is run by default
${BUILD_DIR}/os-image.bin: bootloader ${BUILD_DIR}/kernel/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	mkfs.fat -F 12 -n "EMOS" $@
	dd if=${BUILD_DIR}/boot/stage1/boot.bin of=$@ conv=notrunc
	mcopy -i $@ ${BUILD_DIR}/boot/stage2/boot.bin "::boot2.bin"
	mcopy -i $@ $(BUILD_DIR)/kernel/kernel.bin "::kernel.bin"
	mcopy -i $@ test.txt "::test.txt"


bootloader: ${BUILD_DIR}/boot/stage1/boot.bin
	$(MAKE) -C $(SRC_DIR)/boot/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))


run: ${BUILD_DIR}/os-image.bin
	qemu-system-i386 -fda $<

# Generic rules for wildcards
# To make an object, always compile from its .c
${BUILD_DIR}/%.o: ${SRC_DIR}/%.c ${HEADERS}
	${CC} ${CFLAGS} $< -fo=$@

${BUILD_DIR}/%.o: ${SRC_DIR}/%.asm always
	nasm $< -f obj -o $@

${BUILD_DIR}/%.bin: ${SRC_DIR}/%.asm always
	nasm $< -f bin -o $@

always:
	mkdir -p ${BUILD_DIR}/boot/stage1
	mkdir -p ${BUILD_DIR}/kernel
	mkdir -p ${BUILD_DIR}/drivers


clean:
	rm -rf ${BUILD_DIR}/*.o ${BUILD_DIR}/*.bin ${BUILD_DIR}/*.elf