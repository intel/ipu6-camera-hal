#
#  GStreamer
#  Copyright (C) 2015-2019 Intel Corporation
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
#  Alternatively, the contents of this file may be used under the
#  GNU Lesser General Public License Version 2.1 (the "LGPL"), in
#  which case the following provisions apply instead of the ones
#  mentioned above:
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public
#  License along with this library; if not, write to the
#  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#  Boston, MA 02111-1307, USA.
#

# Note: plugindir is set in configure

SUBDIRS = interfaces

##############################################################################
# TODO: change libgstcamerasrc.la to something else, e.g. libmysomething.la     #
##############################################################################
GST_API_VERSION=1.0
plugin_LTLIBRARIES = libgsticamerasrc.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgsticamerasrc_la_SOURCES = gstcamerasrc.cpp \
                              gstcameraformat.cpp \
                              gstcamerasrcbufferpool.cpp \
                              gstcameradeinterlace.cpp \
                              gstcambasesrc.cpp \
                              gstcampushsrc.cpp \
                              utils.cpp

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsticamerasrc_la_CPPFLAGS = \
    $(GST_CFLAGS) \
    $(CAMHAL_CFLAGS) \
    -std=c++11 \
    -Werror \
    $(LIBUTILS_CFLAGS) \
    -I$(top_srcdir)/src/interfaces \
    -DDEFAULT_PROP_DEVICE_ID=$(DEFAULT_PROP_DEVICE_ID)

if USE_CHROME_SLIM_CAMHAL
libgsticamerasrc_la_CPPFLAGS += -DCHROME_SLIM_CAMHAL
endif

# for hardening-check
libgsticamerasrc_la_CPPFLAGS+= -fstack-protector -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security

libgsticamerasrc_la_LIBADD = $(GST_LIBS) \
    -lgstallocators-$(GST_API_VERSION) \
    -lgstvideo-$(GST_API_VERSION) \
    -L./interfaces/.libs/ -lgsticamerainterface \
    $(CAMHAL_LIBS)

libgsticamerasrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
# for hardening-check
libgsticamerasrc_la_LDFLAGS += -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-z,relro -Wl,-z,now

libgsticamerasrc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstcamerasrc.h \
                 gstcameraformat.h \
                 gstcamerasrcbufferpool.h \
                 gstcameradeinterlace.h \
                 gstcambasesrc.h \
                 gstcampushsrc.h \
                 utils.h
