OUTPUT_ARCH(arm)
ENTRY(_start)

/*
 * Nokia Lumia 520 (Fame) downstream boot-image layout:
 *   base          0x80200000
 *   kernel offset 0x00008000
 *   kernel load   0x80208000
 *   tags offset   0x00000100 -> 0x80200100
 *
 * This is the verified legacy Android/LK-style handoff layout used by the
 * existing Fame Linux port. It is not proof that every SBL3 chain path uses
 * the same address, so direct-SBL3 experiments should still inspect r2/ATAGS.
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

    .stack ALIGN(16) (NOLOAD) :
    {
        __stack_bottom = .;
        . += 16K;
        __stack_top = .;
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
