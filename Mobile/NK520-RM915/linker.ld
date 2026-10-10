OUTPUT_ARCH(arm)
ENTRY(_start)

/*
 * Nokia Lumia 520 (Fame) downstream boot-image layout:
 *   base          0x80200000
 *   kernel offset 0x00008000
 *   kernel load   0x80208000
 *   tags offset   0x00000100 -> 0x80200100
 */
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
     * ARM exception modes have banked stack pointers. Keep dedicated stacks
     * so a fault cannot trample the normal SVC/kernel stack while we capture
     * diagnostics.
     */
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
