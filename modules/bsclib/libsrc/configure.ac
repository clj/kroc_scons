AC_INIT([bsclib], [1.0], [kroc-bugs@kent.ac.uk])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC
OCCAM_OCCBUILD

build_cspdrvlib=no
build_filelib=yes
build_socklib=yes

AC_ARG_ENABLE([cspdrvlib],
              AS_HELP_STRING([--enable-cspdrvlib],
                             [enable cspdrvlib.module; requires kroc and udc.h]),
              [build_cspdrvlib=yes])

dnl Checks for headers
AC_CHECK_HEADER(unistd.h, [], [build_filelib=no])
AC_CHECK_HEADER(sys/socket.h, [], [build_socklib=no])
AC_CHECK_HEADER(sys/sendfile.h, [have_sendfile_h=yes], [have_sendfile_h=no])
if test "$have_sendfile_h" = yes; then
 AC_DEFINE(HAVE_SENDFILE_H)
fi
AC_CHECK_HEADER(sys/uio.h, [have_uio_h=yes], [have_uio_h=no])
if test "$have_uio_h" = yes; then
 AC_DEFINE(HAVE_UIO_H)
fi
OCCAM_DEFS=""
AC_CHECK_FUNC(readv, [have_readv=yes], [have_readv=no])
AC_CHECK_FUNC(writev, [have_writev=yes], [have_writev=no])
if test "$have_readv" = yes; then
 if test "$have_writev" = yes; then
  OCCAM_DEFS="-DHAVE.RWVEC"
  AC_DEFINE(HAVE_UIO_RWVEC)
 fi
fi

dnl Checks for libraries
OCCAM_SOCK_LIBS

if test "x$OCCBUILD_TOOLCHAIN" != "xkroc"; then
  SAVED_CPPFLAGS="$CFLAGS"
  CPPFLAGS="$CPPFLAGS $OCCBUILD_CFLAGS"
  AC_CHECK_HEADER(udc.h, [], [build_cspdrvlib=no])
  CPPFLAGS="$SAVED_CPPFLAGS"
else
  build_cspdrvlib=no
fi

AC_SUBST(OCCAM_DEFS)

if test "$host_os" = "cygwin"; then
 AC_DEFINE(HOSTOS_CYGWIN)
fi

if test "$build_filelib" != "yes"; then
  build_socklib=no
fi

AC_MSG_CHECKING([whether to build cspdrvlib.module])
AC_MSG_RESULT($build_cspdrvlib)
AM_CONDITIONAL(BUILD_CSPDRVLIB, [test $build_cspdrvlib = yes])

AC_MSG_CHECKING([whether to build filelib.module])
AC_MSG_RESULT($build_filelib)
AM_CONDITIONAL(BUILD_FILELIB, [test $build_filelib = yes])

AC_MSG_CHECKING([whether to build socklib.module])
AC_MSG_RESULT($build_socklib)
AM_CONDITIONAL(BUILD_SOCKLIB, [test $build_socklib = yes])

dnl Checks for libraries.
OCCAM_PROVIDE(cspdrv.module, cspdrvlib, none, [test $build_cspdrvlib = yes])
OCCAM_PROVIDE(file.module, filelib, none, [test $build_filelib = yes])
OCCAM_PROVIDE(http.module, httplib, file.module sock.module, [test $build_socklib = yes])
OCCAM_PROVIDE(proc.module, proclib, file.module, [test $build_filelib = yes])
OCCAM_PROVIDE(sock.module, socklib, none, [test $build_socklib = yes])

AC_OUTPUT([Makefile socklib/Makefile filelib/Makefile proclib/Makefile httplib/Makefile cspdrvlib/Makefile])
