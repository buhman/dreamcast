__p1ram_start = ORIGIN(p1ram);
/* reserve bytes for serial program loader */
__p1ram_end = ORIGIN(p1ram) + LENGTH(p1ram) - __stack_reservation;

__text_link_start = ADDR(.text);
__text_link_end = ADDR(.text) + SIZEOF(.text);
__text_load_start = LOADADDR(.text);

__data_link_start = ADDR(.data);
__data_link_end = ADDR(.data) + SIZEOF(.data);
__data_load_start = LOADADDR(.data);

__rodata_link_start = ADDR(.rodata);
__rodata_link_end = ADDR(.rodata) + SIZEOF(.rodata);
__rodata_load_start = LOADADDR(.rodata);

__ctors_link_start = ADDR(.ctors);
__ctors_link_end = ADDR(.ctors) + SIZEOF(.ctors);

__bss_link_start = ADDR(.bss);
__bss_link_end = ADDR(.bss) + SIZEOF(.bss);
