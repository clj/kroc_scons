/*
 *	Type definitions
 *	Copyright (C) 1998 Jim Moores
 *	Modifications (C) 2007 Carl Ritson <cgr@kent.ac.uk>
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

#ifndef UKCTHREADS_TYPES_H
#define UKCTHREADS_TYPES_H

#define true 	1
#define TRUE 	1
#define false 	0
#define FALSE 	0

#define one_if_true(x) ((x) & true)
#define zero_if_false(x) ((x))

typedef unsigned int 	word;
typedef unsigned char 	byte;
#ifndef __cplusplus
typedef word	 	bool;
#endif

#endif /* UKCTHREADS_TYPES_H */

