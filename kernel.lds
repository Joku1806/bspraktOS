ENTRY(_start)
SECTIONS
{
  . = 0x00400000;
  .utext : { build/user/*(.text) }

  . = 0x00500000;
  .urodata : { build/user/*(.rodata*) }

  . = 0x00600000;
  .ubss : { build/user/*(.bss) }

  . = 0x00680000;
  .udata : { build/user/*(.data) }

  . = 0x00100000;
  .init : { *(.init) }

  . = 0x00180000;
  .text : { *(.text) }

  . = 0x00200000;
  .rodata : { *(.rodata*) }

  . = 0x00300000;
  .bss : { *(.bss) }

  . = 0x00380000;
  .data : { *(.data) }
}

