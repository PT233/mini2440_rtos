ENTRY(_start)
SECTIONS
{
    /* 
     * Link address settings
     * For SDRAM debug run: 0x30000000
     * For NOR Flash run: 0x00000000
     */
    . = 0x30000000; 

    .text : {
        *startup.o (.text)  /* Ensure startup code first */
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
