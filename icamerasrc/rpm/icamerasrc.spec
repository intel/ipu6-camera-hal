#
#  GStreamer
#  Copyright (C) 2015-2016 Intel Corporation
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

Summary: Linux icamerasrc
Name: icamerasrc
Version: %{version}
Release: %{release}
License: LGPL
Group: Development/Tools
Prefix: /usr/lib/gstreamer-1.0
BuildRoot: %(mktemp -ud %{_builddir}/%{name}-%{version}-%{release}-XXXXXX)
%description
Linux camera src.

%prep
%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/lib/gstreamer-1.0
mkdir -p %{buildroot}/usr/include/icamerasrc/interfaces
mkdir -p %{buildroot}/usr/include/gstreamer-1.0/gst

cp -v %{_srcdir}/src/.libs/*.so* %{buildroot}/usr/lib/gstreamer-1.0
cp -v %{_srcdir}/src/.libs/*.la  %{buildroot}/usr/lib/gstreamer-1.0
cp -v %{_srcdir}/src/interfaces/.libs/*.so* %{buildroot}/usr/lib/
cp -v %{_srcdir}/src/interfaces/.libs/*.la %{buildroot}/usr/lib/
cp -v %{_srcdir}/src/interfaces/*.h %{buildroot}/usr/include/icamerasrc/interfaces
cp -v %{_srcdir}/src/interfaces/*.h %{buildroot}/usr/include/gstreamer-1.0/gst

%files
/usr/lib/gstreamer-1.0
/usr/lib/
/usr/include/
