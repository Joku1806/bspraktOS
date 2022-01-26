ENTRY(_start)
SECTIONS
{
  . = 0x00080000;
  .utext = { build/user/*(.text) }

  . = 0x00090000;
  .urodata = { build/user/*(.rodata*) }

  . = 0x000c0000;
  .udata = { build/user/*(.data) }

  . = 0x00008000;
  .init : { *(.init) }

  . = 0x00010000;
  .text = { *(.text) }

  . = 0x00020000;
  .rodata = { *(.rodata*) }

  . = 0x00050000;
  .data = { *(.data) }
}

