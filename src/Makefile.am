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

libeom_la_LTLIBRARIES = libeom.la
libeom_ladir = $(libdir)
libeom_la_LIBADD = @LIBEOM_LIBS@
libeom_la_LDFLAGS = ${LDFLAGS}
libeom_la_CFLAGS = \
    ${CFLAGS} \
    @LIBEOM_CFLAGS@ \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src

libeom_la_SOURCES = \
	eom.c

if HAVE_EOM_PLATFORM_X11
libeom_la_CFLAGS += \
	-I$(top_srcdir)/src/dbus

libeom_la_SOURCES += \
	dbus/eom-dbus.c
endif

if HAVE_EOM_PLATFORM_WAYLAND
libeom_la_CFLAGS += \
	-I$(top_srcdir)/src/wayland \
	-I$(top_srcdir)/src/wayland/protocol

libeom_la_SOURCES += \
	wayland/protocol/xdg-shell-protocol.c \
	wayland/protocol/eom-protocol.c \
	wayland/eom-wayland.c
endif



