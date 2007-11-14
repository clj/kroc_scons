// Copyright: Fred Barnes, Mario Schweigler (C) 2000-2006
// Institution: Computing Laboratory, University of Kent, Canterbury, UK
// Description: pony internal TCP/IP socket utilities C code file

/*
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
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *	MA 02110-1301, USA.
 */

//{{{  Compiler declarations
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_UIO_H
#include <sys/uio.h>
#endif

#ifdef HOSTOS_CYGWIN
#include <w32api/wtypes.h>
#include <w32api/winbase.h>
#endif
//}}}

//{{{  Structures
struct __occ_socket {
	int fd;
	int local_port;
	int remote_port;
	int local_addr;
	int remote_addr;
	int s_family;
	int s_type;
	int s_proto;
	int error;
#ifdef __GNUC__
	} __attribute__ ((packed));
#else
	};
	#warning Not GNU C - might get structure problems..
#endif
typedef struct __occ_socket occ_socket;
//}}}

//{{{  Functions

//{{{  `Real' functions

//{{{  Allocation heuristic for `pony_real_fullread_multi'
static int frm_data_mdim = 2048;
static int frm_sizes_mdim = 64;
//}}}

//{{{  static __inline__ void pony_real_fullwrite_iovecdim
// Parameters: *sizes     | size-array
//             sizes_len  | length of size-array
//             *iovec_dim | RETURN VALUE: iovecs dimensions to be allocated
static __inline__ void pony_real_fullwrite_iovecdim (int *sizes, int sizes_len, int *iovec_dim)
{
#ifdef HAVE_UIO_RWVEC
	int nvecs;
	if (sizes_len > 1) {
		nvecs = 2 + sizes_len;		// header, sizes, and all data
	} else {
		nvecs = 2;			// header and single data
	}
	*iovec_dim = nvecs * sizeof (struct iovec);
	return;
#else
	// ... do it the hard way ...
	*iovec_dim = -1;
#endif
	return;
}
//}}}
//{{{  static __inline__ void pony_real_fullwrite_multi
// Parameters: *sock         | socket
//             *header       | header
//             header_len    | length of header
//             *addrs        | address-array
//             addrs_len     | length of address-array
//             *sizes        | size-array
//             sizes_len     | length of size-array
//             *iovecs_param | iovecs to be used
//             iovecs_len    | length of iovecs
//             *result       | RETURN VALUE: result
static __inline__ void pony_real_fullwrite_multi (occ_socket *sock, char *header, int header_len, int *addrs, int addrs_len, int *sizes, int sizes_len, char *iovecs_param, int iovecs_len, int *result)
{
#ifdef HAVE_UIO_RWVEC
	struct iovec *iovecs;
	int nvecs, nbytes, i, j;

	if (sock->fd < 0) {
		*result = -1;
		sock->error = ENOTCONN;
		return;
	}
	if (sizes_len > 1) {
		nvecs = 2 + sizes_len;		// header, sizes, and all data
	} else {
		nvecs = 2;			// header and single data
	}
	iovecs = (struct iovec *)iovecs_param;

	nbytes = header_len;
	iovecs[0].iov_base = (void *)header;
	iovecs[0].iov_len = header_len;
	if (sizes_len > 1) {
		iovecs[1].iov_base = (void *)sizes;
		iovecs[1].iov_len = sizes_len * sizeof(int);
		nbytes += iovecs[1].iov_len;
		i = 2;
	} else {
		i = 1;
	}
	for (j=0; j<sizes_len; j++) {
		iovecs[i+j].iov_base = (void *)(addrs[j]);
		iovecs[i+j].iov_len = sizes[j];
		nbytes += iovecs[i+j].iov_len;
	}

	// enter write() loop
	i = 0;
	j = 0;		// indexes iovecs
	while (i < nbytes) {
		int out;

		out = writev (sock->fd, &(iovecs[j]), nvecs - j);
		if (out < 0) {
			*result = -1;
			sock->error = errno;
			return;
		} else if (out == 0) {
			*result = -1;
			sock->error = 0;
			return;
		}
		// wrote something
		i += out;
		if (i < nbytes) {
			// still got some left
			while ((out > 0) && (iovecs[j].iov_len >= out)) {
				out -= iovecs[j].iov_len;
				j++;
			}
			// adjust
			if (out) {
				iovecs[j].iov_base = ((char *)(iovecs[j].iov_base)) + out;
				iovecs[j].iov_len -= out;
			}
		}
	}
	*result = nbytes;
#else
	// ... do it the hard way ...
	sock->error = ENOSYS;
	*result = -1;
#endif
	return;
}
//}}}
//{{{  static __inline__ void pony_real_fullread_sizes
// Parameters: *data_dim  | RETURN VALUE: data-array dimensions to be allocated
//             *sizes_dim | RETURN VALUE: sizes-array dimensions to be allocated
static __inline__ void pony_real_fullread_sizes (int *data_dim, int *sizes_dim)
{
	*data_dim = frm_data_mdim;
	*sizes_dim = frm_sizes_mdim;
	return;
}
//}}}
//{{{  static __inline__ void pony_real_fullread_multi
// Parameters: *sock         | socket
//             *header       | RETURN VALUE: header
//             header_len    | length of header
//             **data_maddr  | RETURN VALUE: data-array
//             *data_mdim    | RETURN VALUE: dimensions of data-array
//             *data_size    | RETURN VALUE: target dimensions of data-array
//             **sizes_maddr | RETURN VALUE: size-array
//             *sizes_mdim   | RETURN VALUE: dimensions of size-array
//             *sizes_size   | RETURN VALUE: target dimensions of size-array
//             *result       | RETURN VALUE: result
static __inline__ void pony_real_fullread_multi (occ_socket *sock, char *header, int header_len, char **data_maddr, int *data_mdim, int *data_size, int **sizes_maddr, int *sizes_mdim, int *sizes_size, int *result)
{
#ifdef HAVE_UIO_RWVEC
	int in = 0;
	int ccount;
	int dsize = 0;

	if (sock->fd < 0) {
		*result = -1;
		sock->error = ENOTCONN;
		return;
	}
	switch (*result) {
	case 0:
		//{{{  read in the header
		while (in < header_len) {
			int x;

			x = read (sock->fd, header + in, header_len - in);
			if (x < 0) {
				sock->error = errno;
				*result = -1;
				return;
			} else if (x == 0) {
				sock->error = ENOTCONN;
				*result = -1;
				return;
			}
			in += x;
		}
		//}}}
		// fall through to read for sizes
	case -3:
		//{{{  extract comm-count from header and read in sizes (if required)
		ccount = *(int *)(header + (header_len - sizeof (int)));
		if (ccount == 1) {
			int size = *(int *)(header + (header_len - (2 * sizeof(int))));

			// sizes array must be at least 1 big..
			(*sizes_maddr)[0] = size;
			dsize = size;
		} else if (ccount > 1) {
			int toread = (ccount * sizeof(int));

			if (*sizes_mdim < ccount) {
				frm_sizes_mdim = ccount * 2;				// slightly generous
				// sizes array not big enough -- will fit in data array ?
				if (*data_mdim >= (ccount * sizeof(int))) {
					// yes, so swap the buffers around
					int *tmpa = *sizes_maddr;
					int tmpd = *sizes_mdim;

					*sizes_maddr = (int *)(*data_maddr);
					*sizes_mdim = (*data_mdim / sizeof(int));	// gcc will optimise div away..
					*sizes_size = *sizes_mdim;
					*data_maddr = (char *)tmpa;
					*data_mdim = (tmpd * sizeof(int));
					*data_size = *data_mdim;
				} else {
					// nope, will need to re-read sizes
					*result = -3;
					return;
				}
			}
			// if we get this far, can read the sizes in
			in = 0;
			while (in < toread) {
				int x;

				x = read (sock->fd, (char *)(*sizes_maddr) + in, toread - in);
				if (x < 0) {
					sock->error = errno;
					*result = -1;
					return;
				} else if (x == 0) {
					sock->error = ENOTCONN;
					*result = -1;
					return;
				}
				in += x;
			}
			// sum sizes into "dsize"
			dsize = 0;
			for (in=0; in<ccount; in++) {
				dsize += (*sizes_maddr)[in];
			}
		}
		if (ccount == 0) {
			*sizes_size = 0;
		} else {
			*sizes_mdim = ccount;
			*sizes_size = ccount;
		}
		//}}}
		// fall through to read data
	case -4:
		if (dsize == 0) {
			// got here as a result of failed read last time, setup dsize
			ccount = *(int *)(header + (header_len - sizeof (int)));

			if (ccount == 1) {
				dsize = *(int *)(header + (header_len - (2 * sizeof(int))));
			} else {
				for (in=0; in<ccount; in++) {
					dsize += (*sizes_maddr)[in];
				}
			}
		}
		// target array large enough ?
		if (*data_mdim < dsize) {
			// nope
			*result = -4;
			frm_data_mdim = dsize * 2;		// slightly generous
			return;
		}
		in = 0;
		// read data
		while (in < dsize) {
			int x;

			x = read (sock->fd, (*data_maddr) + in, dsize - in);
			if (x < 0) {
				sock->error = errno;
				*result = -1;
				return;
			} else if (x == 0) {
				sock->error = ENOTCONN;
				*result = -1;
				return;
			}
			in += x;
		}
		if (dsize == 0) {
			*data_size = 0;
		} else {
			*data_mdim = dsize;
			*data_size = dsize;
		}
		// right, should be all done..
		*result = header_len + dsize;
                if (ccount > 1) {
                        *result += ccount * sizeof(int);
                }
		break;
	}
#else
	// ... do it the hard way ...
	sock->error = ENOSYS;
	*result = -1;
#endif
	return;
}
//}}}

//{{{  static __inline__ void pony_real_getcode_eaddrinuse
// Parameters: *err_code | RETURN VALUE: error-code
static __inline__ void pony_real_getcode_eaddrinuse (int *err_code)
{
	*err_code = EADDRINUSE;
	return;
}
//}}}

//}}}
//{{{  Interface functions

//{{{  void _pony_int_tcpip_socket_fullwrite_iovecdim
// PROC C.pony.int.tcpip.socket.fullwrite.iovecdim (VAL []INT sizes, RESULT INT iovecdim)
void _pony_int_tcpip_socket_fullwrite_iovecdim (int *ws) {
  pony_real_fullwrite_iovecdim ((int *)(ws[0]), (int)(ws[1]), (int *)(ws[2]));
}
//}}}
//{{{  void _pony_int_tcpip_socket_fullwrite_multi
// PROC B.pony.int.tcpip.socket.fullwrite.multi (SOCKET sock, VAL []BYTE header, VAL []INT addrs, sizes, RESULT []BYTE iovec.buffer, RESULT INT result)
void _pony_int_tcpip_socket_fullwrite_multi (int *ws) {
  pony_real_fullwrite_multi ((occ_socket *)(ws[0]),
                             (char *)(ws[1]), (int)(ws[2]),
                             (int *)(ws[3]), (int)(ws[4]),
                             (int *)(ws[5]), (int)(ws[6]),
                             (char *)(ws[7]), (int)(ws[8]),
                             (int *)(ws[9]));
}
//}}}
//{{{  void _pony_int_tcpip_socket_fullread_sizes
// PROC C.pony.int.tcpip.socket.fullread.sizes (INT data.dim, sizes.dim)
void _pony_int_tcpip_socket_fullread_sizes (int *ws) {
  pony_real_fullread_sizes ((int *)(ws[0]), (int *)(ws[1]));
}
//}}}
//{{{  void _pony_int_tcpip_socket_fullread_multi
//PROC B.pony.int.tcpip.socket.fullread.multi (SOCKET sock, RESULT []BYTE header, MOBILE []BYTE data, INT data.size, MOBILE []INT sizes, INT sizes.size, RESULT INT result)
void _pony_int_tcpip_socket_fullread_multi (int *ws) {
  pony_real_fullread_multi ((occ_socket *)(ws[0]),
                            (char *)(ws[1]), (int)(ws[2]),
                            (char **)(&(ws[3])), (int *)(&(ws[4])), (int *)(ws[5]),
                            (int **)(&(ws[6])), (int *)(&(ws[7])), (int *)(ws[8]),
                            (int *)(ws[9]));
}
//}}}

//{{{  void _pony_int_tcpip_getcode_eaddrinuse
// PROC C.pony.int.tcpip.getcode.eaddrinuse (INT err.code)
void _pony_int_tcpip_getcode_eaddrinuse (int *ws) {
  pony_real_getcode_eaddrinuse ((int *)(ws[0]));
}
//}}}

//}}}

//}}}

