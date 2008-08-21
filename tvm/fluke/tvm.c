/*
 * tvm.c - FLUKE LPC2104 ARM7TDMI TVM Wrapper
 * (formerly: tvm.c - SRV-1 Blackfin TVM Wrapper)
 *
 * Copyright (C) 2008 Jon Simpson, Matthew C. Jadud, Carl G. Ritson
 */

#include "fluke.h"

static tvm_t 		tvm;
static tvm_ectx_t 	firmware_ctx, user_ctx;

static void arm7tdmi_modify_sync_flags (ECTX ectx, WORD set, WORD clear)
{
	/* Fine, until we enable interrupts... */
	ectx->sflags = (ectx->sflags & (~clear)) | set;
}

#if 0
/*{{{  Scheduling support */
static void srv_modify_sync_flags (ECTX ectx, WORD set, WORD clear)
{
	unsigned short imask;
	
	DISABLE_INTERRUPTS (imask);

	ectx->sflags = (ectx->sflags & (~clear)) | set;

	ENABLE_INTERRUPTS (imask);
}

static void clear_pending_interrupts (void)
{
	unsigned short imask;
	WORD flags;
	
	DISABLE_INTERRUPTS (imask);
	
	flags = firmware_ctx.sflags | user_ctx.sflags;

	firmware_ctx.sflags	&= ~(TVM_INTR_SFLAGS);
	user_ctx.sflags		&= ~(TVM_INTR_SFLAGS);

	ENABLE_INTERRUPTS (imask);

	if (flags & TVM_INTR_PPI_DMA) {
		complete_ppi_dma_interrupt (&firmware_ctx);
	}
	if (flags & TVM_INTR_UART0_RX) {
		complete_uart0_rx_interrupt (&firmware_ctx);
	}
	if (flags & TVM_INTR_UART0_TX) {
		complete_uart0_tx_interrupt (&firmware_ctx);
	}
	if (flags & TVM_INTR_TWI) {
		complete_twi_interrupt (&firmware_ctx);
	}
}
/*}}}*/ 
#endif 

static EXT_CHAN_ENTRY	ext_chans[] = {};
/*{{{  External channel definitions */
#if 0
static EXT_CHAN_ENTRY	ext_chans[] = {
	{ 
		.typehash 	= 0,
		.in 		= uart0_in, 
		.out 		= uart0_out,
		.mt_in		= NULL,
		.mt_out		= NULL
	},
	{
		.typehash	= 0,
		.in		= ppi_dma_in,
		.out		= ppi_dma_out,
		.mt_in		= ppi_dma_mt_in,
		.mt_out		= NULL
	},
	{
		.typehash	= 0,
		.in		= twi_in,
		.out		= twi_out,
		.mt_in		= twi_mt_in,
		.mt_out		= twi_mt_out
	}
};
#endif
static const int	ext_chans_length =
				sizeof(ext_chans) / sizeof(EXT_CHAN_ENTRY);
/*}}}*/

/*{{{  User context state */
static BYTEPTR		user_bytecode;
static WORD		user_bytecode_len;
static WORDPTR		user_memory;
static WORD		user_memory_len;
static WORDPTR 		user_parent	= (WORDPTR) NOT_PROCESS_P;
/*}}}*/

/*{{{  Firmware functions for running user bytecode */
/* PROC firmware.run.user (VAL []BYTE bytecode, VAL INT ws, vs, ms, 
 * 				VAL []BYTE tlp, ...) */
static int firmware_run_user (ECTX ectx, WORD args[])
{
	BYTEPTR bytecode	= (BYTEPTR) args[0];
	WORD	bytecode_len	= args[1];
	WORD	ws_size		= args[2];
	WORD	vs_size		= args[3];
	WORD	ms_size		= args[4];
	char	*tlp_fmt	= (char *) wordptr_real_address ((WORDPTR) args[5]);
	WORD	argc		= args[6];
	WORDPTR	argv		= (WORD *) &(args[7]);
	WORDPTR	ws, vs, ms;
	WORD	ret_addr;
	int ret;

	if (user_parent != (WORDPTR) NOT_PROCESS_P) {
		/* User context is already running */
		return ectx->set_error_flag (ectx, EFLAG_FFI);
	}

	tvm_ectx_reset (&user_ctx);
	user_memory_len = tvm_ectx_memory_size (
		&user_ctx, tlp_fmt, argc, 
		ws_size, vs_size, ms_size
	);
	user_memory = (WORDPTR) tvm_malloc (ectx, user_memory_len << WSH);
	tvm_ectx_layout (
		&user_ctx, user_memory, 
		tlp_fmt, argc, 
		ws_size, vs_size, ms_size,
		&ws, &vs, &ms
	);
	ret = tvm_ectx_install_tlp (
		&user_ctx, bytecode, ws, vs, ms,
		tlp_fmt, argc, argv
	);
	if (ret) {
		/* Install TLP failed */
		return ectx->set_error_flag (ectx, EFLAG_FFI);
	}

	/* Save bytecode addresses */
	user_bytecode		= bytecode;
	user_bytecode_len	= bytecode_len;

	/* Simulate return, and deschedule */
	ret_addr	= read_word (ectx->wptr);
	/* Push WPTR up 4 words */
	user_parent 	= wordptr_plus (ectx->wptr, 4);
	/* Store return address as descheduled IPTR */
	WORKSPACE_SET (user_parent, WS_IPTR, ret_addr);
	/* Save execution context for good measure */
	WORKSPACE_SET (user_parent, WS_ECTX, (WORD) ectx);

	return SFFI_RESCHEDULE;
}

/* PROC firmware.kill.user () */
static int firmware_kill_user (ECTX ectx, WORD args[])
{
	if (user_parent != (WORDPTR) NOT_PROCESS_P)
	{
		/* Restore parent in firmware */
		firmware_ctx.add_to_queue (&firmware_ctx, user_parent);
		user_parent = (WORDPTR) NOT_PROCESS_P;
		/* Disconnect any top-level channels. */
		tvm_ectx_disconnect (&user_ctx);
	}

	return SFFI_OK;
}

/* PROC firmware.query.user (BOOL running, INT state, []BYTE context) */
static int firmware_query_user (ECTX ectx, WORD args[])
{
	WORDPTR running = (WORDPTR) args[0];
	WORDPTR state	= (WORDPTR) args[1];
	BYTEPTR	ctx	= (BYTEPTR) args[2];
	WORD	ctx_len	= args[3];
	BYTE 	*uctx	= (BYTE *) &user_ctx;
	int i;

	/* BOOL/WORD running */
	write_word (running, user_parent != (WORDPTR) NOT_PROCESS_P ? 1 : 0);
	/* WORD state */
	write_word (state, (WORD) user_ctx.state);
	/* []BYTE context */
	for (i = 0; i < ctx_len && i < sizeof(user_ctx); ++i)
	{
		write_byte (ctx, *(uctx++));
		ctx = byteptr_plus (ctx, 1);
	}

	return SFFI_OK;
}

#if 0
/* PROC reset.dynamic.memory () */
static int reset_dynamic_memory (ECTX ectx, WORD args[])
{
	#ifdef TVM_USE_TLSF
	const UWORD dmem_length = SDRAM_TOP - DMEM_START;

	tlsf_init_memory_pool (dmem_length, ectx->mem_pool);

	return SFFI_OK;
	#else
	return ectx->set_error_flag (ectx, EFLAG_FFI);
	#endif
}
#endif

#if 0
/* PROC safe.set.register.16 (INT16 reg, VAL INT set, clear) */
static int set_register_16 (ECTX ectx, WORD args[])
{
	volatile unsigned short *addr	= (unsigned short *) args[0];
	unsigned short set		= (unsigned short) args[1];
	unsigned short clear		= (unsigned short) args[2];
	unsigned short imask;

	DISABLE_INTERRUPTS (imask);

	*addr = ((*addr) & (~clear)) | set;

	ENABLE_INTERRUPTS (imask);

	return SFFI_OK;
}
#endif

/* PROC test.disconnected (CHAN ANY c, BOOL b) */
static int test_disconnected (ECTX ectx, WORD args[])
{
	WORDPTR	chan_ptr	= (WORDPTR) args[0];
	WORDPTR	out		= (WORDPTR) args[1];

	write_word (out, (read_word (chan_ptr) == (NOT_PROCESS_P | 1)) ? 1 : 0);

	return SFFI_OK;
}
/*}}}*/

/*{{{  SFFI tables */
static SFFI_FUNCTION	firmware_sffi_table[] = {
	firmware_run_user,
	firmware_kill_user,
	firmware_query_user,
	NULL, /* reset_dynamic_memory,*/
	/*set_register_16,
	jpeg_encode_frame,
	draw_caption_on_frame,
	test_disconnected*/
};
static const int	firmware_sffi_table_length =
				sizeof(firmware_sffi_table) / sizeof(SFFI_FUNCTION);

static SFFI_FUNCTION	user_sffi_table[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	/* set_register_16,
	jpeg_encode_frame,
	draw_caption_on_frame,
	test_disconnected*/
};
static const int	user_sffi_table_length =
				sizeof(user_sffi_table) / sizeof(SFFI_FUNCTION);
/*}}}*/

/*{{{  Firmware context */
#define transputercode __attribute__ ((section (".rodata"))) transputercode
#include "firmware.h"
#undef transputercode

static WORD firmware_memory[304];

static void init_firmware_memory (void)
{
	WORD *ptr = firmware_memory;
	int words = (sizeof(firmware_memory) / sizeof(WORD));
	
	while ((words--) > 0) {
		*(ptr++) = MIN_INT;
	}
}

static void install_firmware_ctx (void)
{
	WORDPTR ws, vs, ms;
	ECTX firmware = &firmware_ctx;

	/* Initialise firmware execution context */
	tvm_ectx_init (&tvm, firmware);
	firmware->get_time 		= arm7tdmi_get_time;
	firmware->modify_sync_flags	= arm7tdmi_modify_sync_flags;
	firmware->ext_chan_table	= ext_chans;
	firmware->ext_chan_table_length	= ext_chans_length;
	firmware->sffi_table		= firmware_sffi_table;
	firmware->sffi_table_length	= firmware_sffi_table_length;
	/* Dynamic memory */
	#ifdef TVM_USE_TLSF
	firmware->mem_pool		= (void *) DMEM_START;
	#endif
	
	/* Setup memory and initial workspace */
	init_firmware_memory ();
	tvm_ectx_layout (
		firmware, firmware_memory,
		"", 0, ws_size, vs_size, ms_size, 
		&ws, &vs, &ms
	);
	tvm_ectx_install_tlp (
		firmware, (BYTEPTR) transputercode, ws, vs, ms, 
		"", 0, NULL
	);
}

static int run_firmware (void)
{
	int ret;

	do {
		ret = tvm_run (&firmware_ctx);

		if (ret == ECTX_SLEEP) {
			return ret; /* OK - timer sleep */
		} else if (ret == ECTX_EMPTY) {
#if 0		
			if (uart0_is_blocking ()) {
				return ret; /* OK - waiting for input */
			} else if (ppi_dma_is_blocking ()) {
				return ret; /* OK - waiting for imagery */
			} else if (twi_is_blocking ()) {
				return ret; /* OK - waiting for TWI transaction */
			} else if (user_parent != (WORDPTR) NOT_PROCESS_P) {
#endif
			if (user_parent != (WORDPTR) NOT_PROCESS_P) { /* copied from line above without else */
				if (user_ctx.state == ECTX_EMPTY && user_ctx.fptr == (WORDPTR) NOT_PROCESS_P) {
					if (tvm_ectx_waiting_on (&user_ctx, user_memory, user_memory_len)) {
						/* User code is waiting on us so we are probably
						 * in the wrong; bail...
						 */
					} else {
						/* User code is not waiting on us, so spin and
						 * let it get deadlock detected, if killing it
						 * doesn't release us then we we'll be back
						 * here...
						 */
						return ret;
					}
				} else {
					/* Optimise for the common case by ignoring 
					 * the possibility of deadlock when the
					 * user code can still keep running.
					 */
					return ret;
				}
			}
			/* Fall through indicates deadlock */
		} else if (ret == ECTX_INTERRUPT) {
#if 0
			clear_pending_interrupts ();
#endif
			/* OK; fall through and loop */
		}
	} while (ret == ECTX_INTERRUPT);

	/* Being here means something unexpected happened... */
	
	debug_print_str ("## Firmware failed; state = ");
	debug_print_chr (ret);
	debug_print_str (", eflags = ");
	debug_print_hex (firmware_ctx.eflags);
	debug_print_chr ('\n');

	if (user_parent != (WORDPTR) NOT_PROCESS_P) {
		debug_print_str ("## User state = ");
		debug_print_chr (user_ctx.state);
		debug_print_str (", eflags = ");
		debug_print_hex (user_ctx.eflags);
		debug_print_chr ('\n');
	}

	/* Go into an idle loop */
	for (;;) {
#if 0
		IDLE;
		SSYNC;
#endif
	}

	return ret;
}
/*}}}*/

/*{{{  User context */
static void install_user_ctx (void)
{
	ECTX user = &user_ctx;

	tvm_ectx_init (&tvm, user);
	user->get_time 			= arm7tdmi_get_time;
	user->modify_sync_flags		= arm7tdmi_modify_sync_flags;
	user->sffi_table		= user_sffi_table;
	user->sffi_table_length		= user_sffi_table_length;
	
	/* Dynamic memory */
	#ifdef TVM_USE_TLSF
	user->mem_pool			= (void *) DMEM_START;
	#endif
}

static int run_user (void)
{
	int ret = tvm_run_count (&user_ctx, 1000);

	switch (ret) {
		case ECTX_INTERRUPT:
#if 0
			clear_pending_interrupts ();
#endif
			/* fall through */
		case ECTX_PREEMPT:
		case ECTX_SLEEP:
		case ECTX_TIME_SLICE:
			return ret; /* OK */
		case ECTX_EMPTY:
			if (tvm_ectx_waiting_on (&user_ctx, user_memory, user_memory_len)) {
				return ret; /* OK - waiting for firmware */
			}
			break;
		default:
			break;
	}

	/* User context broke down for some reason. */
	/* Restore parent in firmware */
	firmware_ctx.add_to_queue (&firmware_ctx, user_parent);
	user_parent = (WORDPTR) NOT_PROCESS_P;

	/* Disconnect any top-level channels. */
	tvm_ectx_disconnect (&user_ctx);

	return ECTX_ERROR;
}

static void tvm_sleep (void)
{
	ECTX firmware	= &firmware_ctx;
	ECTX user	= &user_ctx;
	WORD is_timed	= 1;
	WORD timeout;

	if (firmware->state == ECTX_SLEEP && user->state == ECTX_SLEEP) {
		if (TIME_AFTER (user->tnext, firmware->tnext)) {
			timeout	= firmware->tnext;
		} else {
			timeout	= user->tnext;
		}
	} else if (firmware->state == ECTX_SLEEP) {
		timeout	= firmware->tnext;
	} else if (firmware->state == ECTX_SLEEP) {
		timeout	= user->tnext;
	} else {
		is_timed = timeout = 0;
	}

	if (is_timed) {
		sleep_until (timeout);
	} else {
		sleep ();
	}
}
/*}}}*/

/*{{{  Interfacing */
int tvm_interrupt_pending (void)
{
	return (firmware_ctx.sflags | user_ctx.sflags);
}

void raise_tvm_interrupt (WORD flag)
{
	WORD flags = SFLAG_INTR | flag;

	firmware_ctx.sflags	|= flags;
	user_ctx.sflags		|= flags;
}

int run_tvm (void)
{
	/* Initialise interpreter */
	tvm_init (&tvm);
	install_firmware_ctx ();
	install_user_ctx ();

	/* Run interpreter */
	for (;;) {
		int f_ret = run_firmware ();
		int u_ret = ECTX_EMPTY;

		if (user_parent != (WORDPTR) NOT_PROCESS_P) {
			u_ret = run_user ();
		}

		if ((f_ret == ECTX_EMPTY || f_ret == ECTX_SLEEP) && 
			(u_ret == ECTX_EMPTY || u_ret == ECTX_SLEEP)) {
			if (firmware_ctx.fptr == NOT_PROCESS_P && user_ctx.fptr == NOT_PROCESS_P) {
				tvm_sleep ();
			}
		}
	}

	return 1;
}
/*}}}*/

