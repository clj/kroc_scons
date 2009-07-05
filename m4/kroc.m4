#
#	KRoC definitions for autoconf
#	Copyright (C) 2009 Adam Sampson <ats@offog.org>
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
dnl
dnl Get settings for compiling code against CCSP.
AC_DEFUN([KROC_CCSP_FLAGS],
[dnl
AC_REQUIRE([AC_CANONICAL_SYSTEM])
AC_REQUIRE([OCCAM_IN_TREE])

KROC_CCSP_CFLAGS=""
KROC_CCSP_CINCPATH=""
KROC_CCSP_ASFLAGS=""
KROC_CCSP_OCCFLAGS=""
KROC_CCSP_TRANFLAGS=""
KROC_CCSP_LDFLAGS=""
KROC_CCSP_LIBPATH=""
KROC_CCSP_LIBS=""

KROC_CCSP_ENABLE_PTHREADS=""
KROC_CCSP_ENABLE_MP=""
KROC_CCSP_ENABLE_CTTD=""
KROC_CCSP_ENABLE_PONY=""
KROC_CCSP_ENABLE_DYNPROC=""

if test "x$KROC_BUILD_ROOT" != "x"; then
  # We're configuring inside the KRoC source tree; we need to figure out the
  # flags based on the target and configure options.

  AC_ARG_ENABLE([pthreads],
                AS_HELP_STRING([--enable-pthreads],
                               [enable pthreads support (default enabled)]),
                KROC_CCSP_ENABLE_PTHREADS=$enableval,
                KROC_CCSP_ENABLE_PTHREADS=yes)
  AC_ARG_ENABLE([mp],
                AS_HELP_STRING([--disable-mp],
                               [disable multiprocessor support (default enabled)]),
                KROC_CCSP_ENABLE_MP=$enableval,
                KROC_CCSP_ENABLE_MP=yes)
  AC_ARG_ENABLE([cttd],
                AS_HELP_STRING([--enable-cttd],
                               [enable CHAN TYPE type description support (default disabled)]),
                KROC_CCSP_ENABLE_CTTD=$enableval,
                KROC_CCSP_ENABLE_CTTD=no)
  AC_ARG_ENABLE([pony],
                AS_HELP_STRING([--enable-pony],
                               [enable pony transparent networking (default disabled)]),
                KROC_CCSP_ENABLE_PONY=$enableval,
                KROC_CCSP_ENABLE_PONY=no)
  AC_ARG_ENABLE([dynproc],
                AS_HELP_STRING([--enable-dynproc],
                               [enable dynamic process loading (default enabled)]),
                KROC_CCSP_ENABLE_DYNPROC=$enableval,
                KROC_CCSP_ENABLE_DYNPROC=yes)

  KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -fomit-frame-pointer -fno-defer-pop"

  case "$target_cpu" in
    x86_64)
      # Compile in 32-bit mode.
      KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -m32"
      KROC_CCSP_ASFLAGS="$KROC_CCSP_ASFLAGS --32"
      KROC_CCSP_LDFLAGS="$KROC_CCSP_LDFLAGS -m32 -Wl,-melf_i386"
      ;;
  esac

  case "$target_os" in
    cygwin*)
      KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DHOSTOS_CYGWIN"
      ;;
    darwin*)
      # Disable automatic PIC usage on Apple's GCC.
      KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DHOSTOS_DARWIN -mdynamic-no-pic"
      ;;
  esac

  # If we're configuring in the tree, we also need to get our headers and
  # libraries from there.
  KROC_CCSP_CINCPATH="$KROC_CCSP_CINCPATH -I$KROC_BUILD_ROOT/runtime/ccsp/include -I$KROC_SRC_ROOT/runtime/ccsp/include -I$KROC_BUILD_ROOT/modules/cif/libsrc -I$KROC_SRC_ROOT/modules/cif/libsrc"
  KROC_CCSP_LIBPATH="$KROC_CCSP_LIBPATH -L$KROC_BUILD_ROOT/runtime/ccsp -L$KROC_BUILD_ROOT/runtime/libkrocif"

  AC_CHECK_FUNC(dlsym, have_libc_dlsym=yes, have_libc_dlsym=no)
  AC_CHECK_LIB(dl, dlsym, have_libdl=yes, have_libdl=no)
  if test $have_libc_dlsym = yes; then
    :
  elif test $have_libdl = yes; then
    KROC_CCSP_LIBS="$KROC_CCSP_LIBS -ldl"
  else
    KROC_CCSP_ENABLE_DYNPROC=no
  fi

  AC_CHECK_LIB(elf, elf_begin, have_libelf=yes, have_libelf=no)
  if test $have_libelf = yes; then
    KROC_CCSP_LIBS="$KROC_CCSP_LIBS -lelf"
  fi

  AC_CHECK_LIB(pthread, pthread_create, have_libpthread=yes, have_libpthread=no)

  # On Solaris, sem_init is in librt.
  AC_CHECK_LIB(rt, sem_init, have_librt_sem=yes, have_librt_sem=no)

  # On FreeBSD, you need to pass -pthread to the compiler when building
  # threaded programs.
  SAVED_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -pthread"
  AC_CHECK_FUNC(pthread_create, have_dashpthread=yes, have_dashpthread=no)
  CFLAGS="$SAVED_CFLAGS"

  AC_MSG_CHECKING([whether to use POSIX threads])
  if test $KROC_CCSP_ENABLE_PTHREADS = yes; then
    if test $have_libpthread = yes; then
      AC_MSG_RESULT(yes)
      KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DKROC_USES_PTHREADS"
      KROC_CCSP_LDFLAGS="$KROC_CCSP_LDFLAGS -lpthread"
      if test $have_librt_sem = yes; then
        KROC_CCSP_LDFLAGS="$KROC_CCSP_LDFLAGS -lrt"
      fi
    elif test $have_dashpthread = yes; then
      AC_MSG_RESULT(yes)
      KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DKROC_USES_PTHREADS"
      KROC_CCSP_LDFLAGS="$KROC_CCSP_LDFLAGS -pthread"
    else
      AC_MSG_RESULT(no)
    fi
  else
    AC_MSG_RESULT(no)
  fi

  # For multiprocessor support, we must have either pthreads or RMoX.
  if test $KROC_CCSP_ENABLE_PTHREADS = yes; then
    :
  elif test $KROC_CCSP_ENABLE_RMOX = yes; then
    :
  else
    KROC_CCSP_ENABLE_MP=no
  fi

  AC_MSG_CHECKING([whether to enable multiprocessor support])
  if test $KROC_CCSP_ENABLE_MP = yes; then
    AC_MSG_RESULT(yes)
    KROC_CCSP_TRANFLAGS="$KROC_CCSP_TRANFLAGS -mp"
  else
    AC_MSG_RESULT(no)
  fi

  AC_MSG_CHECKING([what level of CHAN TYPE type descriptions to support])
  if test $KROC_CCSP_ENABLE_PONY = yes; then
    AC_MSG_RESULT(pony)
    KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DCHANTYPEDESC -DCHANTYPEUIO -DCHANTYPESTATE"
    KROC_CCSP_OCCFLAGS="$KROC_CCSP_OCCFLAGS -zctt -zctuio -zctknsf"
    KROC_CCSP_TRANFLAGS="$KROC_CCSP_TRANFLAGS --cttd"
  elif test $KROC_CCSP_ENABLE_CTTD = yes; then
    AC_MSG_RESULT(cttd)
    KROC_CCSP_CFLAGS="$KROC_CCSP_CFLAGS -DCHANTYPEDESC"
    KROC_CCSP_OCCFLAGS="$KROC_CCSP_OCCFLAGS -zctt"
    KROC_CCSP_TRANFLAGS="$KROC_CCSP_TRANFLAGS --cttd"
  else
    AC_MSG_RESULT(none)
  fi

  AC_MSG_CHECKING([whether to enable dynamic process loading])
  AC_MSG_RESULT($KROC_CCSP_ENABLE_DYNPROC)

else
  # We're not in the KRoC source tree, so we can just call kroc to get the
  # arguments.
  AC_MSG_ERROR(FIXME: builds outside source tree not implemented)
fi

AC_SUBST(KROC_CCSP_CFLAGS)
AC_SUBST(KROC_CCSP_CINCPATH)
AC_SUBST(KROC_CCSP_OCCFLAGS)
AC_SUBST(KROC_CCSP_TRANFLAGS)
AC_SUBST(KROC_CCSP_ASFLAGS)
AC_SUBST(KROC_CCSP_LDFLAGS)
AC_SUBST(KROC_CCSP_LIBPATH)
AC_SUBST(KROC_CCSP_LIBS)
])dnl
dnl
dnl Find the "kroc" script, and define KROC.
AC_DEFUN([KROC_PROG_KROC],
[dnl
AC_REQUIRE([OCCAM_IN_TREE])
AC_ARG_VAR(KROC, [Path to kroc])
if test "x$KROC_BUILD_ROOT" != "x"; then
  KROC="$KROC_BUILD_ROOT/tools/kroc/kroc --in-tree $KROC_BUILD_ROOT"
else
  AC_CHECK_PROG(KROC, kroc, kroc, no)
  if test $KROC = no; then
    AC_MSG_ERROR([kroc not found; set \$KROC or \$PATH appropriately])
  fi
fi
])dnl
