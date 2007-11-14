/*
 *	bsyscalls.h
 *	Blocking sys-call interface definition (architecture specific)
 *	Copyright (C) 2000 F.R.M. Barnes, J. Moores and P.H. Welch
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __BSYSCALLS_H
#define __BSYSCALLS_H

#define NUM_CLONES		(3)
#define MAX_NUM_CLONES		(4096)
#define CLONE_STACK_SIZE	(256 * 4096)		/* 1 MB stack should be ample.. */

typedef struct TAG_bsc_thread {
	int		*ws_ptr;			/* Wptr of process executing (already adjusted for return) */
	int		*ws_arg;			/* argument to func */
	void		*ws_arg2, *ws_arg3;		/* two additional arguments, for 3-arg things (needed for user-defined-channels) */
	int		arg3;				/* indicates to use 3-argument call (ws_arg, ws_arg2, ws_arg3) */
	void		(*func)(word *);		/* function to execute */
	char		*raddr;				/* return address */
	int		adjustment;			/* are we terminateable ? (or offset of workspace params) */
#ifdef PROCESS_PRIORITY
	int		pri;				/* priority of invoking process */
#endif
#ifdef BLOCKING_PROFILE
	long long	time_dispatch;			/* time when dispatched */
#endif
#if defined(PROCESS_PRIORITY) && defined(BLOCKING_PROFILE)
	int		padding[5];			/* makes state 16 words */
#elif defined(PROCESS_PRIORITY)
	int		padding[7];			/* makes state 16 words */
#elif defined(BLOCKING_PROFILE)
	int		padding[6];			/* makes state 16 words */
#else
	/* state is 8 words */
#endif

	/* all the above are communicated during dispatch, those below are private */
	int		pid;				/* process ID */
	int		thr_num;			/* thread number */
	int		par_sync;			/* pipe for synchronisation */

	int		terminated;			/* set when terminated */

	int		cancel;				/* set when cancelling */
	sigjmp_buf	*jbuf;				/* ptr to jump buffer - (allocated just beyond this structure) */
	void		*user_ptr;			/* ptr to where the user can store state (just beyond `jbuf') */

	void		(*clone_cleanup)(void *);	/* ptr to clean-up function */

#ifdef BLOCKING_PROFILE
	long long	time_kill;			/* time when signalled to kill */
#endif
} bsc_thread __attribute__ ((packed));

#define bsc_commsize(X)  ((int)(&((X).pid)) - (int)(&(X)))

#ifdef BLOCKING_PROFILE
	#define NR_BUCKETS		8193		/* last used for overflow */
	#define LAST_BUCKET		(NR_BUCKETS - 1)
	#define BUCKET_RESOLUTION	5		/* divide time in cycles by 32 */

	#define ll_lo(X)	((int *)&(X))[0]
	#define ll_hi(X)	((int *)&(X))[1]

	#define LOADTIME(time) \
		__asm__ __volatile__ ("\n\t" \
			".byte 0x0f,0x31\n\t" \
			: "=a" (ll_lo(time)), "=d" (ll_hi(time)));

	#ifndef __BSYSCALLS_C
		extern volatile int bp_dispatch_trigger[NR_BUCKETS];
		extern volatile int bp_trigger_finish[NR_BUCKETS];
		extern volatile int bp_kill_finish[NR_BUCKETS];
		extern volatile int bp_finish_collect[NR_BUCKETS];
	#endif
	#define SHIFTDIFFERENCE(first,second) (int)((second - first) >> BUCKET_RESOLUTION)
	#define ADDTOBUCKET(bucket,first,second) { \
		int value = SHIFTDIFFERENCE(first,second); \
		if (value >= LAST_BUCKET) { \
			bucket[LAST_BUCKET]++; \
		} else if (value < 0) { \
			bucket[LAST_BUCKET]++; \
		} else { \
			bucket[value]++; \
		} \
	}
#endif


#endif	/* !__BSYSCALLS_H */
