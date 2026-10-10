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
        KEEP(*(.text.context_switch))
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

    .mmu_tables ALIGN(16K) (NOLOAD) :
    {
        __mmu_l1_start = .;
        KEEP(*(.mmu_tables))
        __mmu_l1_end = .;
    }

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
