
#include "tvm_posix.h"

/* PROC init.input () */
static int init_input (ECTX ectx, WORD args[])
{
	init_terminal ();
	atexit (restore_terminal);
	return SFFI_OK;
}

/* PROC read.char (RESULT INT ch) */
static int _read_char (ECTX ectx, WORD args[])
{
	WORDPTR ch = (WORDPTR) args[0];

	/* Return -1 if nothing was available, and the character otherwise */
	if (char_available()) {
		write_word (ch, read_char());
	} else {
		write_word (ch, -1);
	}

	return SFFI_OK;
}

/* PROC write.screen (VAL []BYTE buffer) */
static int write_screen (ECTX ectx, WORD args[])
{
	char *buffer	= (char *) wordptr_real_address ((WORDPTR) args[0]);
	WORD length	= args[1];

	if (length > 0) {
		fwrite (buffer, length, 1, stdout);
	}

	fflush (stdout);
	
	return SFFI_OK;
}

/* PROC write.error (VAL BYTE ch) */
static int write_error (ECTX ectx, WORD args[])
{
	fputc (args[0], stderr);
	fflush (stderr);
	return SFFI_OK;
}

/* PROC get_argv (INT argc, INT argv) */
static int get_argv (ECTX ectx, WORD args[])
{
	WORDPTR	argc	= (WORDPTR) args[0];
	WORDPTR	argv	= (WORDPTR) args[1];

	write_word (argc, tvm_argc - 1);
	write_word (argv, (WORD) &(tvm_argv[1]));
	
	return SFFI_OK;
}

/* PROC get.version ([]BYTE str) */
static int get_version (ECTX ectx, WORD args[])
{
	BYTEPTR	str	= (BYTEPTR) args[0];
	WORD	str_len	= args[1];

	strncpy ((char *) wordptr_real_address ((WORDPTR) str), VERSION, str_len);
	
	return SFFI_OK;
}

static SFFI_FUNCTION sffi_table[] = {
	init_input,
	_read_char,
	write_screen,
	write_error,
	get_argv,
	get_version
};

void install_sffi (ECTX ectx)
{
	ectx->sffi_table	= sffi_table;
	ectx->sffi_table_length	= sizeof(sffi_table) / sizeof(SFFI_FUNCTION);
}

