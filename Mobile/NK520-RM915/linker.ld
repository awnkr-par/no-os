OUTPUT_ARCH(arm)
ENTRY(_start)

KERNEL_LOAD = 0x80208000;

SECTIONS
{
    . = KERNEL_LOAD;
    __image_start = .;

    .vectors ALIGN(32) :
    {
        KEEP(*(.vectors))
    }

    .text ALIGN(4) :
    {
        *(.text.boot)
        *(.text .text.*)
    }

    .rodata ALIGN(4) :
    {
        *(.rodata .rodata.*)
    }

    .data ALIGN(4) :
    {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    }

    .bss ALIGN(8) (NOLOAD) :
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /*
     * Deliberately outside .bss so startup clearing does not erase the block
     * before C explicitly initializes it. This also gives debuggers a stable
     * linker symbol and section to locate after a failed boot.
     */
    .noos_diag ALIGN(64) (NOLOAD) :
    {
        __diag_start = .;
        KEEP(*(.noos_diag))
        __diag_end = .;
    }

    .stacks ALIGN(16) (NOLOAD) :
    {
        __svc_stack_bottom = .;
        . += 16K;
        __svc_stack_top = .;

        __irq_stack_bottom = .;
        . += 4K;
        __irq_stack_top = .;

        __fiq_stack_bottom = .;
        . += 4K;
        __fiq_stack_top = .;

        __abt_stack_bottom = .;
        . += 4K;
        __abt_stack_top = .;

        __und_stack_bottom = .;
        . += 4K;
        __und_stack_top = .;
    }

    __image_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
