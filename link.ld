ENTRY(loader)               /* entry label */

SECTIONS {
    . = 0xC0100000;         /* code should be loaded at 3 GiB + 1 MiB */
    
    /* these labels get exported to the code files */
    kernel_virtual_start = .;
    kernel_physical_start = . - 0xC0000000;

    .__mb_header : AT(ADDR(.__mb_header) - 0xC0000000)
    {
        *(.__mb_header)
    }

    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000) /* align at 4 kiB and load at 1 MiB*/
    {
        *(.text)            /* all text sections from all files */
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)         /* all ready-only data sections from all files */
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)            /* all data sections from all files */
    }

    .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)           /* all COMMON sections from all files */
        *(.bss)             /* all bss sections from all files */
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - 0xC0000000;
}