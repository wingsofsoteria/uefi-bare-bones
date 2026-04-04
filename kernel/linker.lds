ENTRY(_start)

SECTIONS
{
	. = 0xFFFF800000000000;

	.text : AT(ADDR(.text) - 0xFFFF800000000000)
	{
		*(.text)
		. = ALIGN(4096);
	}

	.rodata : AT(ADDR(.rodata) - 0xFFFF800000000000)
	{
		*(.rodata)
		. = ALIGN(4096);
	}

	.data : AT(ADDR(.data) - 0xFFFF800000000000)
	{
		*(.data)
		. = ALIGN(4096);
	}

	.bss : AT(ADDR(.bss) - 0xFFFF800000000000)
	{
		*(COMMON)
		*(.bss)
		. = ALIGN(4096);
	}
	.kernel_init : AT(ADDR(.kernel_init) - 0xFFFF800000000000)
	{
	_kernel_init_start_ = .;
	*(.kernel_init)
	_kernel_init_end_ = .;
	. = ALIGN(4096);
	}
}
