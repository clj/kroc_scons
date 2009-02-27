#
#	occbuild definitions for autoconf
#	Copyright (C) 2007 University of Kent
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
dnl Determine whether we're building a package in the KRoC tree.
dnl Set KROC_BUILD_ROOT to the top of the tree if we are, and to
dnl the empty string if we aren't.
AC_DEFUN([OCCAM_IN_TREE],
[dnl
AC_ARG_VAR(KROC_BUILD_ROOT, [Top of KRoC source tree])

AC_MSG_CHECKING([whether we're building in the KRoC source tree])

# Try to find the top-level configure.ac for KRoC.
old_PWD=`pwd`
KROC_BUILD_ROOT=""
for try in 1 2 3 4 5 6; do
	if test -f configure.ac && grep '^# KROC_IS_ROOT' configure.ac >/dev/null; then
		KROC_BUILD_ROOT=`pwd`
		break
	fi
	cd ..
done
cd $old_PWD

if test "x$KROC_BUILD_ROOT" != "x"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
])dnl
dnl
dnl Determine the toolchain we're using (based on configure arguments), and set
dnl OCCBUILD_TOOLCHAIN.
AC_DEFUN([OCCAM_TOOLCHAIN],
[dnl
OCCBUILD_TOOLCHAIN=kroc
AC_ARG_WITH([toolchain],
            AS_HELP_STRING([--with-toolchain=ENV],
                           [select occam toolchain to use (kroc, tvm; default kroc)]),
            [OCCBUILD_TOOLCHAIN="$withval"])
AM_CONDITIONAL(OCCBUILD_KROC, test "x$OCCBUILD_TOOLCHAIN" = "xkroc")
AM_CONDITIONAL(OCCBUILD_TVM, test "x$OCCBUILD_TOOLCHAIN" = "xtvm")
])dnl
dnl
dnl Find occbuild.
AC_DEFUN([OCCAM_OCCBUILD],
[dnl
AC_REQUIRE([OCCAM_IN_TREE])
AC_REQUIRE([OCCAM_TOOLCHAIN])
AC_ARG_VAR(OCCBUILD, [Path to occbuild])
if test "x$KROC_BUILD_ROOT" != "x"; then
  OCCBUILD="$KROC_BUILD_ROOT/tools/kroc/occbuild --in-tree $KROC_BUILD_ROOT"
else
  AC_CHECK_PROG(OCCBUILD, occbuild, occbuild, no)
  if test $OCCBUILD = no; then
    AC_MSG_ERROR([occbuild not found; set \$OCCBUILD or \$PATH appropriately])
  fi
fi
dnl
OCCBUILD="$OCCBUILD --toolchain=$OCCBUILD_TOOLCHAIN"
dnl
if test "x$KROC_BUILD_ROOT" != "x"; then
  if test "x$OCCBUILD_TOOLCHAIN" = "xkroc"; then
    KROC_CCSP_FLAGS
    OCCBUILD_CFLAGS="-DOCCBUILD_KROC $KROC_CCSP_CFLAGS $KROC_CCSP_CINCPATH"
  elif test "x$OCCBUILD_TOOLCHAIN" = "xtvm"; then
    OCCBUILD_CFLAGS="-DOCCBUILD_TVM"
  else
    AC_MSG_ERROR([don't know how to find OCCBUILD_CFLAGS in-tree for this toolchain])
  fi
else
  AC_MSG_CHECKING([for flags needed when compiling FFI objects])
  OCCBUILD_CFLAGS=$($OCCBUILD --cflags)
  AC_MSG_RESULT([$OCCBUILD_CFLAGS])
fi
AC_SUBST(OCCBUILD_CFLAGS)
])dnl
dnl
dnl Find occamdoc.
AC_DEFUN([OCCAM_OCCAMDOC],
[dnl
AC_REQUIRE([OCCAM_IN_TREE])
AC_ARG_VAR(OCCAMDOC, [Path to occamdoc])
if test "x$KROC_BUILD_ROOT" != "x"; then
  OCCAMDOC="$KROC_BUILD_ROOT/tools/occamdoc/occamdoc --in-tree $KROC_BUILD_ROOT"
else
  AC_CHECK_PROG(OCCAMDOC, occamdoc, occamdoc, no)
  if test $OCCAMDOC = no; then
    AC_MSG_ERROR([occamdoc not found; set \$OCCBUILD or \$PATH appropriately])
  fi
fi
])dnl
dnl
dnl Check for the presence of occam modules (or other include files).
dnl OCCAM_INCLUDE(FILES, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
AC_DEFUN([OCCAM_INCLUDE],
[dnl
AC_REQUIRE([OCCAM_IN_TREE])
AC_REQUIRE([OCCAM_OCCBUILD])

AC_MSG_CHECKING([for occam include files $1])
if test "x$KROC_BUILD_ROOT" != "x"; then
  # In-tree build: look up those files in in-tree-modules.
  AC_MSG_RESULT([maybe])
  found=yes
  touch $KROC_BUILD_ROOT/in-tree-modules
  find_module () {
    want_file=[$]1
    AC_MSG_CHECKING([for in-tree include file $want_file])
    found_this=no
    while read file path deps; do
      if test "$file" = "$want_file" && test "$path" != "-"; then
        OCCBUILD="$OCCBUILD --search $path"
        found_this=yes
        found_deps="$deps"
        break
      fi
    done <$KROC_BUILD_ROOT/in-tree-modules
    if test $found_this = yes; then
      if test "x$found_deps" = "x"; then
        AC_MSG_RESULT(yes)
      else
        AC_MSG_RESULT([yes, and it needs $found_deps])
      fi
    else
      AC_MSG_RESULT(no)
      found=no
    fi
    for dep in $found_deps; do
      find_module $dep
    done
  }
  for want_file in $1; do
    find_module $want_file
  done
  AC_MSG_CHECKING([whether we found $1])
else
  # Out-of-tree build: try compiling a program that uses those files.
  : >conftest.occ
  for file in $1; do
    printf >>conftest.occ '#INCLUDE "%s"\n' "$file"
  done
  printf >>conftest.occ 'PROC main ()\n  SKIP\n:\n'
  if AC_RUN_LOG([$OCCBUILD --object conftest.occ >/dev/null]); then
    found=yes
  else
    found=no
  fi
fi

if test $found = yes; then
  AC_MSG_RESULT([yes])
  OCCAM_INCLUDED="$OCCAM_INCLUDED $1"
  $2
else
  AC_MSG_RESULT([no])
  $3
fi
AC_RUN_LOG([$OCCBUILD --clean conftest.tce])
rm -f conftest.occ
])dnl
dnl
dnl Record that this package provides some modules (or other include files).
dnl If the module is in a subdirectory, pass that as SUBDIRECTORY.
dnl By default, the dependencies of FILES will be recorded as any files you've
dnl checked for using OCCAM_INCLUDE already; if the dependencies are different,
dnl pass them as DEPENDENCIES. You can pass "none" as DEPENDENCIES to force the
dnl modules to have no dependencies.
dnl If the module may only be built sometimes (e.g. if it depends on external
dnl libraries that may not be present on some systems), provide a shell command
dnl as CONDITION that can be used as an "if" test.
dnl OCCAM_PROVIDE(FILES, [SUBDIRECTORY], [DEPENDENCIES], [CONDITION])
AC_DEFUN([OCCAM_PROVIDE],
[dnl
AC_REQUIRE([OCCAM_IN_TREE])

if test "x$KROC_BUILD_ROOT" != "x"; then
  if test "x$2" = "x"; then
    dir=`pwd`
  else
    dir=`pwd`/$2
  fi
  if test "x$3" = "x"; then
    deps="$OCCAM_INCLUDED"
  elif test "x$3" = "xnone"; then
    deps=""
  else
    deps="$3"
  fi
  if test "x$4" = "x"; then
    condition=true
  else
    condition="$4"
  fi
  touch $KROC_BUILD_ROOT/in-tree-modules
  for file in $1; do
    (grep -v "^$file " $KROC_BUILD_ROOT/in-tree-modules; if $condition; then echo "$file $dir $deps"; else echo "$file -"; fi) | sort >$KROC_BUILD_ROOT/in-tree-modules.new
    mv -f $KROC_BUILD_ROOT/in-tree-modules.new $KROC_BUILD_ROOT/in-tree-modules
  done
fi
])dnl
dnl
AC_DEFUN([OCCAM_SWIG],
[dnl
HAVE_SWIG_OCCAM=no
AC_CHECK_PROG(SWIG, swig, swig, no)
if test $SWIG != no; then
	SWIG_OCCAM="$SWIG -occampi"
	AC_SUBST([SWIG_OCCAM])
	dnl
	# Check if the -occampi flag is supported
	AC_MSG_CHECKING([for SWIG occam-pi support])
	touch conftest.i
	if $SWIG_OCCAM -module conftest conftest.i >/dev/null 2>&1; then
		AC_MSG_RESULT([yes])
		HAVE_SWIG_OCCAM=yes
		$1
	else
		AC_MSG_RESULT([no])
		$2
	fi
fi
AM_CONDITIONAL(HAVE_SWIG_OCCAM, test "x$HAVE_SWIG_OCCAM" = "xyes")
rm -f conftest.i
])dnl
