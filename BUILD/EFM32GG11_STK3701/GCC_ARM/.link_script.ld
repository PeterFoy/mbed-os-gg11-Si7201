MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x200000
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 524288
}
__vector_size = 0x158;
ENTRY(Reset_Handler)
SECTIONS
{
  .text :
  {
    KEEP(*(.vectors))
    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - __Vectors;
    __end__ = .;
    *(.text*)
    KEEP(*(.init))
    KEEP(*(.fini))
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)
    *(.rodata*)
    KEEP(*(.eh_frame*))
  } > FLASH
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH
  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH
  __exidx_end = .;
  __etext = .;
  .data : AT (__etext)
  {
    __data_start__ = .;
    PROVIDE( __start_vector_table__ = .);
    . += __vector_size;
    PROVIDE( __end_vector_table__ = .);
    *(vtable)
    *(.data*)
    . = ALIGN (8);
    *(.ram)
    . = ALIGN(8);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(8);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(8);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    KEEP(*(.jcr*))
    . = ALIGN(8);
    __data_end__ = .;
  } > RAM
  .bss :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(8);
    __bss_end__ = .;
  } > RAM
  .heap (COPY):
  {
    __HeapBase = .;
    __end__ = .;
    end = __end__;
    _end = __end__;
    KEEP(*(.heap*))
    __HeapLimit = .;
  } > RAM
  .stack_dummy (COPY):
  {
    KEEP(*(.stack*))
  } > RAM
  __StackTop = ORIGIN(RAM) + LENGTH(RAM);
  __StackLimit = __StackTop - SIZEOF(.stack_dummy);
  PROVIDE(__stack = __StackTop);
  ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
  ASSERT(ORIGIN(FLASH) + LENGTH(FLASH) >= (__etext + SIZEOF(.data)), "FLASH memory overflowed !")
}
