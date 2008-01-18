/*
 *	CCSP compiler support macros
 *	Copyright (C) 2008  Carl Ritson <cgr@kent.ac.uk>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __COMPILER_H
#define __COMPILER_H

/*{{{  likely/unlikely optimisation */
#ifdef __GNUC__
#define likely(X)       __builtin_expect((X),1)
#define unlikely(X)     __builtin_expect((X),0)
#endif
/*}}}*/

#endif	/* !__COMPILER_H */

