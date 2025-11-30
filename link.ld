ENTRY(_start)
SECTIONS
{
    /* 
     * 设置链接地址
     * 如果是在 SDRAM 中调试运行，0x30000000
     * 如果是烧录到 NOR Flash 运行，0x00000000
     */
    . = 0x30000000; 

    .text : {
        *startup.o (.text) /* 确保 startup 代码在最前面 */
        *(.text)
        *(.rodata)
    }

    . = ALIGN(4);
    _etext = .;

    .data : {
        _data = .;
        *(.data)
    }

    . = ALIGN(4);
    _bss_start_ = .;
    .bss : {
        *(.bss)
        *(COMMON)
    }
    _bss_end_ = .;
}
