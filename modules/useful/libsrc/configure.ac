AC_INIT([useful], [1.0], [kroc-bugs@kent.ac.uk])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC
OCCAM_OCCBUILD
AM_PATH_PYTHON([2.2], [], [exit 1])

AC_MSG_CHECKING([trace mechanism to use])
trace_mechanism=ffi
case "$host_cpu" in
  avr*)
    trace_mechanism=none
    ;;
esac
AC_MSG_RESULT([$trace_mechanism])

OCCAM_DEFS=""
case "$trace_mechanism" in
  ffi)
    OCCAM_DEFS="$OCCAM_DEFS -DTRACE.VIA.FFI"
    ;;
esac
AC_SUBST(OCCAM_DEFS)
AM_CONDITIONAL([TRACE_VIA_FFI], [test $trace_mechanism = ffi])

dnl Checks for occam modules.
OCCAM_INCLUDE([course.module maths.module], ,
              [AC_MSG_ERROR(Required modules not found)])
OCCAM_PROVIDE(useful.module)

AC_OUTPUT([Makefile])
