ENTRY(__start)

SECTIONS
{
  .init 0x80003100 : { *(.init) *(.init.*) }
  .text 0x800032A0 : { *(.text) *(.text.*) }
  .rodata 0x80104920 : { *(.rodata) *(.rodata.*) }
  .data 0x80123BA0 : { *(.data) *(.data.*) }
  .bss 0x8016BB20 : { *(.bss) *(.bss.*) *(COMMON) }
  .sdata 0x803D2260 : { *(.sdata) *(.sdata.*) }
  .sbss 0x803D3BE0 : { *(.sbss) *(.sbss.*) }
  .data5 0x803D4BE0 : { *(.data5) *(.data5.*) }

  _SDA_BASE_ = 0x803DA260;
  _SDA2_BASE_ = 0x803EA260;
  _stack_end = 0x803D5DA0;
  _stack_addr = 0x803E5DA0;
  __ArenaLo = 0x803E5DC0;
  __ArenaHi = 0x81780000;
}
