# =====================================================================
# Makefile for mini2440_rtos (GNU Toolchain)
# =====================================================================

# 交叉编译器
CROSS_COMPILE ?= arm-none-eabi-

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
READELF = $(CROSS_COMPILE)readelf

# 目标文件名
TARGET  = mini2440_rtos
BUILD_DIR = build

# 编译选项
# -mcpu=arm920t : 指定处理器架构
# -g            : 生成调试信息
# -Wall         : 开启所有警告
# -O0           : 不优化
# -nostdlib     : 不链接标准库
# -MMD -MP      : 生成依赖文件
INCLUDES = -Iinclude -Iarch/inc -Iuser/inc -Idrivers/inc -Ikernel
CFLAGS   = -mcpu=arm920t -g -Wall -O0 -nostdlib $(INCLUDES) -MMD -MP
ASFLAGS  = -mcpu=arm920t -g $(INCLUDES) -Wa,-Iinclude -Wa,-Iarch/inc
LDFLAGS  = -T link.ld

# 源文件列表
SRCS_ASM = $(wildcard arch/*.S)

SRCS_C   = $(wildcard user/*.c)
SRCS_C  += $(wildcard arch/*.c)
SRCS_C  += $(wildcard drivers/*.c)
SRCS_C  += $(wildcard kernel/*.c)
SRCS_C  += $(wildcard lib/*.c)

# QEMU 版本使用不同的启动代码
SRCS_ASM_QEMU = arch/startup_qemu.S $(filter-out arch/startup.S, $(SRCS_ASM))

# 生成文件到build目录
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS_ASM:.S=.o) $(SRCS_C:.c=.o))
OBJS_QEMU = $(addprefix $(BUILD_DIR)/, $(SRCS_ASM_QEMU:.S=.o) $(SRCS_C:.c=.o))
DEPS = $(OBJS:.o=.d)

.PHONY: all clean dis qemu

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).dis $(BUILD_DIR)/$(TARGET).txt

# QEMU 专用版本 (链接到 0x00000000)
qemu: $(BUILD_DIR)/$(TARGET)_qemu.bin

# 1. 链接生成 ELF 文件
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -nostdlib -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -T link.ld -o $@ $^ -lgcc
	@echo
	@echo ">>> ELF Header Info:"
	$(READELF) -h $@
	@echo

# 1b. QEMU 专用 ELF (链接到 0x00000000，使用 startup_qemu.S)
$(BUILD_DIR)/$(TARGET)_qemu.elf: $(OBJS_QEMU)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -nostdlib -Wl,-Map=$(BUILD_DIR)/$(TARGET)_qemu.map -T link_qemu.ld -o $@ $^ -lgcc
	@echo
	@echo ">>> QEMU ELF Header Info:"
	$(READELF) -h $@
	@echo

# 2. 生成bin
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary -S $< $@

# 2b. 生成 QEMU bin
$(BUILD_DIR)/$(TARGET)_qemu.bin: $(BUILD_DIR)/$(TARGET)_qemu.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary -S $< $@

# 3. 生成dis
$(BUILD_DIR)/$(TARGET).dis: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	$(OBJDUMP) -D $< > $@

# 4. 生成详细的符号表和源码混合反汇编
$(BUILD_DIR)/$(TARGET).txt: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	$(OBJDUMP) -x -S $< > $@

# 5. 编译汇编
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

# 6. 编译 C
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# 引入依赖文件
-include $(DEPS)

# 清理规则
clean:
	rm -rf $(BUILD_DIR)

