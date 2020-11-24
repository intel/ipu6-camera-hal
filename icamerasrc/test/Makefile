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

ifndef PKG_CONFIG
PKG_CONFIG=pkg-config
endif

ifndef PRE_BUILD_DIR
PRE_BUILD_DIR=/usr/
endif

ifndef SDKTARGETSYSROOT
GST_CFLAGS=`$(PKG_CONFIG) --cflags gstreamer-1.0 glib-2.0 gstreamer-video-1.0`
GST_LIBS=`$(PKG_CONFIG) --libs  gstreamer-1.0 gobject-2.0  glib-2.0 gstreamer-video-1.0 gstreamer-app-1.0 gstreamer-allocators-1.0`
endif

CXXFLAGS += -g -I./utils/ \
          `$(PKG_CONFIG) --cflags libcamhal` \
          $(GST_CFLAGS) \
          -I../src/interfaces/ \
          -I${PRE_BUILD_DIR}/include \
          -std=c++11

ifeq ($(ENABLE_IPU_SHARED), ON)
LDFLAGS += -L${IPUSHARED_INSTALL_DIR}/lib -Wl,-rpath-link,${IPUSHARED_INSTALL_DIR}/lib -lipu_shared
endif
LDFLAGS += -L${PRE_BUILD_DIR}/lib/ \
         `$(PKG_CONFIG) --libs libcamhal` \
         -Wl,-rpath-link,${AIQ_INSTALL_DIR}/lib -Wl,-rpath-link,${IACSS_INSTALL_DIR}/lib \
         $(GST_LIBS) \
         -L../src/interfaces/.libs/ -lgsticamerainterface \
         -Wl,-Bstatic -lgtest -lgmock -Wl,-Bdynamic -lgcc_s -lpthread

#hardening-check
CXXFLAGS+=-fstack-protector -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security
LDFLAGS+= -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-z,relro -Wl,-z,now
ifeq ($(CHROME_SLIM_CAMHAL), ON)
CXXFLAGS+= -DCHROME_SLIM_CAMHAL
endif

casedir=cases/api
testdir=test

srcobjs=$(patsubst %.cpp, %.o, $(shell ls $(casedir)/*.cpp ))

mainobj=main.o

testbin=icamsrc_test

all: $(testbin)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(testbin): $(mainobj) $(srcobjs)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(mainobj) $(srcobjs) $(testbin) *.yuv *nv12
