/*
tvm - mem_array.h
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

#ifndef MEM_ARRAY_H
#define MEM_ARRAY_H

void setup_mem(WORD *ptr, int size);

WORD read_mem(POOTER ptr);
void write_mem(POOTER ptr, WORD val);
static inline POOTER pooter_minus(POOTER ptr, WORD inc) {
	return ptr - inc * TVM_WORD_LENGTH;
}
static inline POOTER pooter_plus(POOTER ptr, WORD inc) {
	return ptr + inc * TVM_WORD_LENGTH;
}

BYTE read_byte(BPOOTER ptr);
void write_byte(BPOOTER ptr, BYTE val);
static inline BPOOTER bpooter_minus(BPOOTER ptr, WORD inc) {
	return ptr - inc;
}
static inline BPOOTER bpooter_plus(BPOOTER ptr, WORD inc) {
	return ptr + inc;
}

#ifdef __FPU_SUPPORT__

float read_memf(POOTER ptr);
void write_memf(POOTER ptr, float val);

double read_memd(POOTER ptr);
void write_memd(POOTER ptr, double val);

#endif

int *pooter_real_address(POOTER ptr);

#endif

