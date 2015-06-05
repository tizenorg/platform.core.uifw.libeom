# Copyright 2014 Samsung Electronics co., Ltd. All Rights Reserved.
#
# Contact:
# SooChan Lim <sc1.lim@samsung.com>
# Boram Park <boram1288.park@samsung.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

AC_PREREQ(2.60)
AC_INIT(libeom, 1.0.0)
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([dist-bzip2])

AM_CONFIG_HEADER([config.h])
AH_TOP([
#ifndef __EOM_CONFIG_H__
#define __EOM_CONFIG_H__
])
AH_BOTTOM([
#endif /* __EOM_CONFIG_H__ */
])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CC

AC_HEADER_STDC
AC_SYS_LARGEFILE
AC_FUNC_ALLOCA

# Enable quiet compiles on automake 1.11.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for pkg-config packages
PKG_CHECK_MODULES(DBUS, dbus-1)
PKG_CHECK_MODULES(GLIB, glib-2.0 gobject-2.0)
PKG_CHECK_MODULES(ELEMENTARY, elementary)

LIBEOM_CFLAGS="$DBUS_CFLAGS $GLIB_CFLAGS $ELEMENTARY_CFLAGS"
LIBEOM_LIBS="$DBUS_LIBS $GLIB_LIBS $ELEMENTARY_LIBS"

PKG_CHECK_MODULES(CAPI, [capi-base-common >= 0.1.1], [CAPI_VER="yes"], [CAPI_VER="no"])
if test "x$CAPI_VER" = "xyes"; then
    AC_DEFINE(HAVE_CAPI_0_1_1,1,[The version of capi-base-common is over 0.1.1])
    LIBEOM_CFLAGS="$LIBEOM_CFLAGS $CAPI_CFLAGS"
    LIBEOM_LIBS="$LIBEOM_LIBS $CAPI_LIBS"
fi

AC_CHECK_LIB(dlog, __dlog_print, [have_dlog="yes"], [have_dlog="no"])
if test "x$have_dlog" = "xyes"; then
	PKG_CHECK_MODULES(DLOG, dlog)
	AC_DEFINE([HAVE_DLOG], 1, "Have dlog support")
    LIBEOM_CFLAGS="$LIBEOM_CFLAGS $DLOG_CFLAGS"
    LIBEOM_LIBS="$LIBEOM_LIBS $DLOG_LIBS"
fi

AC_SUBST([LIBEOM_CFLAGS])
AC_SUBST([LIBEOM_LIBS])

# Checks for header files.
AC_HEADER_STDC

AC_OUTPUT([Makefile
           src/Makefile
           include/Makefile
           include/eom.pc
           tests/Makefile])

echo "CFLAGS       : $CFLAGS"
echo "LDFLAGS      : $LDFLAGS"
echo "LIBEOM_CFLAGS: $LIBEOM_CFLAGS"
echo "LIBEOM_LIBS  : $LIBEOM_LIBS"
