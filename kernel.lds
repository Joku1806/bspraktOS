ENTRY(_start)
SECTIONS
{
  . = 0x00600000;
  .utext : { build/user/*(.text) }

  . = 0x00700000;
  .urodata : { build/user/*(.rodata*) }

  . = 0x00800000;
  .udata : { build/user/*(.data) }

  . = 0x00100000;
  .init : { *(.init) }

  . = 0x00200000;
  .text : { *(.text) }

  . = 0x00300000;
  .bss : {*(.bss)}

  . = 0x00400000;
  .rodata : { *(.rodata*) }

  . = 0x00500000;
  .data : { *(.data) }
}

