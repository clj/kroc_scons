/*
tvm - scheduler.h
The Transterpreter - a portable virtual machine for Transputer bytecode
Copyright (C) 2004-2008 Christian L. Jacobsen, Matthew C. Jadud

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#define ADD_TO_QUEUE(WS) \
	do { ectx->add_to_queue (ectx, (WS)); } while (0)

#define ADD_TO_QUEUE_IPTR(WS,IP) \
	do { 							\
		WORKSPACE_SET ((WS), WS_IPTR, (WORD)(IP)); 	\
		ADD_TO_QUEUE ((WS));				\
	} while (0)

#define RUN_NEXT_ON_QUEUE() \
	do {							\
		int ret = ectx->run_next_on_queue (ectx);	\
		if (ret)					\
			return ret;				\
	} while (0)

#define RUN_NEXT_ON_QUEUE_RET() \
	do { return ectx->run_next_on_queue (ectx); } while (0)

#define TIMER_QUEUE_INSERT(WS,NOW,RESCHED) \
	do { ectx->timer_queue_insert (ectx, (WS), (NOW), (RESCHED)); } while (0)

extern void _tvm_install_scheduler (ECTX ectx);

#endif
