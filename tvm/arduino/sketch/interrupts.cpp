#include "tvm-arduino.h"

static int num_waiting = 0;

#define NUM_INTERRUPTS 2
typedef struct _vinterrupt {
	WORDPTR wptr;
	WORD pending;
} vinterrupt;
static vinterrupt interrupts[NUM_INTERRUPTS];

void init_interrupts () {
	for (int i = 0; i < NUM_INTERRUPTS; i++) {
		interrupts[i].wptr = (WORDPTR) NOT_PROCESS_P;
		interrupts[i].pending = MIN_INT;
	}
}

// This runs in interrupt context, so no need to cli()/sei().
static void raise_tvm_interrupt (WORD flag) {
	context.sflags |= SFLAG_INTR | flag;
}

static void handle_interrupt (vinterrupt *intr, WORD flag) {
	WORD now = millis ();
	if (intr->wptr != (WORDPTR) NOT_PROCESS_P) {
		WORDPTR ptr = (WORDPTR) WORKSPACE_GET (intr->wptr, WS_POINTER);
		write_word (ptr, now);
		WORKSPACE_SET (intr->wptr, WS_POINTER, NULL_P);
		raise_tvm_interrupt (flag);
	} else {
		if (now == (WORD) MIN_INT) {
			++now;
		}
		intr->pending = now;
	}
}

static void clear_interrupt (vinterrupt *intr) {
	context.add_to_queue (&context, intr->wptr);
	intr->wptr = (WORDPTR) NOT_PROCESS_P;
	--num_waiting;
}

static int wait_interrupt (vinterrupt *intr, ECTX ectx, WORDPTR time_ptr) {
	bool reschedule;

	cli ();

	if (intr->pending != (WORD) MIN_INT) {
		write_word (time_ptr, intr->pending);
		intr->pending = MIN_INT;
		reschedule = false;
	} else {
		// Simulate a return -- since we want to be rescheduled
		// *following* the FFI call.
		// FIXME This should be a macro (it's also used in srv1).
		WORD ret_addr = read_word (ectx->wptr);
		ectx->wptr = wordptr_plus (ectx->wptr, 4);
		WORKSPACE_SET (ectx->wptr, WS_IPTR, ret_addr);
		WORKSPACE_SET (ectx->wptr, WS_ECTX, (WORD) ectx);

		WORKSPACE_SET (ectx->wptr, WS_POINTER, (WORD) time_ptr);
		intr->wptr = ectx->wptr;

		++num_waiting;
		reschedule = true;
	}

	sei ();

	if (reschedule) {
		return SFFI_RESCHEDULE;
	} else {
		return SFFI_OK;
	}
}

extern "C" {
	ISR(TIMER1_OVF_vect) {
		static int ticks = 0;

		ticks++;
		if (ticks % 20 == 0) {
			handle_interrupt (&interrupts[0], TVM_INTR_TIMER1);
		}
	}
	ISR(TIMER2_OVF_vect) {
		handle_interrupt (&interrupts[1], TVM_INTR_TIMER2);
	}
}

void clear_pending_interrupts () {
	if ((context.sflags & TVM_INTR_TIMER1) != 0) {
		clear_interrupt (&interrupts[0]);
	}
	if ((context.sflags & TVM_INTR_TIMER2) != 0) {
		clear_interrupt (&interrupts[1]);
	}

	cli ();
	context.sflags &= ~TVM_INTR_SFLAGS;
	sei ();
}

bool waiting_on_interrupts () {
	return (num_waiting > 0);
}

int ffi_wait_for_interrupt (ECTX ectx, WORD args[]) {
	WORD interrupt = args[0];
	WORDPTR time_ptr = (WORDPTR) args[1];

	if (interrupt < 0 || interrupt >= NUM_INTERRUPTS) {
		return ectx->set_error_flag (ectx, EFLAG_FFI);
	}

	return wait_interrupt (&interrupts[interrupt], ectx, time_ptr);
}
