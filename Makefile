# 交叉编译器
CROSS_COMPILE ?= arm-none-eabi-

# 编译参数
CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
READELF = $(CROSS_COMPILE)readelf

# 目标文件名
TARGET  = mini2440_rtos
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/.obj

# 编译优化选项 (快速编译，保留调试符号)
INCLUDES = -Iinclude
CFLAGS   = -mcpu=arm920t -g -O0 -Wall -nostdlib $(INCLUDES) -MMD -MP
ASFLAGS  = -mcpu=arm920t -g $(INCLUDES) -Wa,-Iinclude
LDFLAGS  = -T link.ld

# 源文件列表
SRCS_ASM = $(wildcard arch/*.S)
SRCS_C   = $(wildcard user/*.c)
SRCS_C  += $(wildcard arch/*.c)
SRCS_C  += $(wildcard drivers/*.c)
SRCS_C  += $(wildcard kernel/*.c)
SRCS_C  += $(wildcard lib/*.c)

# 生成文件到build/.obj目录
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS_ASM:.S=.o) $(SRCS_C:.c=.o))
DEPS = $(OBJS:.o=.d)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).dis $(BUILD_DIR)/$(TARGET).map

# 1. 链接生成 ELF 文件
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	@echo "LD  $@"
	@$(CC) $(CFLAGS) -nostdlib -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -T link.ld -o $@ $^ -lgcc
	@echo
	@echo ">>> Build Complete"

# 2. 生成bin
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary -S $< $@

# 3. 生成 dis
$(BUILD_DIR)/$(TARGET).dis: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	@echo "OBJDUMP $@"
	@$(OBJDUMP) -S $< > $@

# 4. 编译汇编
$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "AS  $<"
	@$(CC) $(ASFLAGS) -c -o $@ $<

# 5. 编译 C
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC  $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

# 引入依赖文件
-include $(DEPS)

# 清理规则
clean:
	rm -rf $(BUILD_DIR)

# 调试相关
debug: all
	gdb-multiarch -x gdbinit.txt build/$(TARGET).elf

# OpenOCD 方式
openocd:
	@echo "Starting OpenOCD..."
	@echo "NOTE: Connect hardware if not already connected"
	openocd -f $(PWD)/DEBUG/openocd.cfg -s $(PWD)/DEBUG/OpenOCD || true

.PHONY: debug jlink openocd kill-openocd clean all