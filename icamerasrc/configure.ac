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

dnl required version of autoconf
AC_PREREQ([2.53])

dnl TODO: fill in your package name and package version here
AC_INIT([my-plugin-package],[1.0.0])

dnl required versions of gstreamer and plugins-base
GST_REQUIRED=1.0.0
GSTPB_REQUIRED=1.0.0

AC_CONFIG_HEADERS([config.h])

dnl required version of automake
AM_INIT_AUTOMAKE([1.10])

dnl enable mainainer mode by default
AM_MAINTAINER_MODE([enable])

dnl check for tools (compiler etc.)
AC_PROG_CXX

dnl required version of libtool
LT_PREREQ([2.2.6])
LT_INIT

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl conditionals to support bitbake builds
AC_ARG_WITH([headercheck],
  [AS_HELP_STRING([--with-headercheck], [check that headers exist] @@)],
  [],
  [with_headercheck=yes])

AC_ARG_WITH([libcheck],
  [AS_HELP_STRING([--with-libcheck], [check that hal lib exists] @@)],
  [],
  [with_libcheck=yes])

AC_ARG_WITH([androidstubs],
  [AS_HELP_STRING([--with-androidstubs], [add android stubs for linking] @@)],
  [],
  [with_androidstubs=no])

AC_ARG_VAR([DEFAULT_CAMERA],
  [the default camera ID])

dnl set license and copyright notice
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [GStreamer license])
AC_SUBST(GST_LICENSE)

dnl set API Version
GST_API_VERSION=1.0
AC_DEFINE_UNQUOTED(GST_API_VERSION, "$GST_API_VERSION", [GStreamer API Version])
AC_SUBST(GST_API_VERSION)

dnl set Package Origin
GST_PACKAGE_ORIGIN="https://developer.gnome.org/gstreamer/"
AC_DEFINE_UNQUOTED(GST_PACKAGE_ORIGIN, "$GST_PACKAGE_ORIGIN", [Gstreamer Package Origin])
AC_SUBST(GST_PACKAGE_ORIGIN)

dnl set Package Name
GST_PACKAGE_NAME="Gstreamer"
AC_DEFINE_UNQUOTED(GST_PACKAGE_NAME, "$GST_PACKAGE_NAME", [Gstreamer Package Name])
AC_SUBST(GST_PACKAGE_NAME)

AC_LANG_PUSH([C++])

# default camera selection
AS_IF([test "x${DEFAULT_CAMERA}" == x],
  [
    AC_SUBST([DEFAULT_PROP_DEVICE_ID], [0])
  ],
  [
    AC_SUBST([DEFAULT_PROP_DEVICE_ID], ["${DEFAULT_CAMERA}"])
  ])

# check for android stub linking
AS_IF([test "x$with_androidstubs" == xyes],
  [
    PKG_CHECK_MODULES([LIBUTILS], [libutils])
  ], [])

AC_LANG_POP([C++])

dnl Check for the required version of GStreamer core (and gst-plugins-base)
dnl This will export GST_CFLAGS and GST_LIBS variables for use in Makefile.am
dnl
dnl If you need libraries from gst-plugins-base here, also add:
dnl for libgstaudio-1.0: gstreamer-audio-1.0 >= $GST_REQUIRED
dnl for libgstvideo-1.0: gstreamer-video-1.0 >= $GST_REQUIRED
dnl for libgsttag-1.0: gstreamer-tag-1.0 >= $GST_REQUIRED
dnl for libgstpbutils-1.0: gstreamer-pbutils-1.0 >= $GST_REQUIRED
dnl for libgstfft-1.0: gstreamer-fft-1.0 >= $GST_REQUIRED
dnl for libgstinterfaces-1.0: gstreamer-interfaces-1.0 >= $GST_REQUIRED
dnl for libgstrtp-1.0: gstreamer-rtp-1.0 >= $GST_REQUIRED
dnl for libgstrtsp-1.0: gstreamer-rtsp-1.0 >= $GST_REQUIRED
dnl etc.
PKG_CHECK_MODULES(GST, [
  gstreamer-1.0 >= $GST_REQUIRED
  gstreamer-base-1.0 >= $GST_REQUIRED
  gstreamer-controller-1.0 >= $GST_REQUIRED
], [
  AC_SUBST(GST_CFLAGS)
  AC_SUBST(GST_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the GStreamer development
      packages on your system. On debian-based systems these are
      libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev.
      on RPM-based systems gstreamer1.0-devel, libgstreamer1.0-devel
      or similar. The minimum version required is $GST_REQUIRED.
  ])
])

PKG_CHECK_MODULES(CAMHAL, [libcamhal], [
  AC_SUBST(CAMHAL_CFLAGS)
  AC_SUBST(CAMHAL_LIBS)
], [
  AC_MSG_ERROR([Cannot find libcamhal pkgconfig])
])

dnl check if compiler understands -Wall (if yes, add -Wall to GST_CFLAGS)
AC_MSG_CHECKING([to see if compiler understands -Wall])
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([ ], [ ])], [
  GST_CFLAGS="$GST_CFLAGS -Wall"
  AC_MSG_RESULT([yes])
], [
  AC_MSG_RESULT([no])
])

dnl set the plugindir where plugins should be installed (for src/Makefile.am)
if test "x${prefix}" = "x$HOME"; then
  plugindir="$HOME/.gstreamer-1.0/plugins"
else
  plugindir="\$(libdir)/gstreamer-1.0"
fi
AC_SUBST(plugindir)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

AC_MSG_CHECKING([check if CHROME_SLIM_CAMHAL is ON])
AS_IF([test "x${CHROME_SLIM_CAMHAL}" = "xON" ], AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))
AM_CONDITIONAL([USE_CHROME_SLIM_CAMHAL], [test "x${CHROME_SLIM_CAMHAL}" = "xON"])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/interfaces/Makefile
                ])
AC_OUTPUT

