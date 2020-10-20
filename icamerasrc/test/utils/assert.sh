#!/bin/bash

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

errno=0
suc_count=0
fail_count=0

function EXPECT_COMPARE() {
  operator_type=$1

  if [ $operator_type = "UNARY_OPERATOR" ] ; then
    expected=
    operator=$2
    actual=$3
    line=$4
  elif [ $operator_type = "BINARY_OPERATOR" ] ; then
    expected=$4
    operator=$3
    actual=$2
    line=$5
  elif [ $operator_type = "TRUE_OPERATOR" ] ; then
    expected=$2
    actual=
    operator=
    line=$3
  fi

  if [ $errno -ne 0 ] ; then
    return 0
  fi

  if [ ! $expected $operator $actual ] ; then
    if [ -n "$line" ]; then
      LOGE Expection Failed at func $func at $line
    else
      LOGE Expection Failed at func $func
    fi
    LOGE Value of Expected $expected
    LOGE Operator $operator
    LOGE Value of actual $actual
    errno=1
  else
    errno=0
  fi;
}

function EXPECT_TRUE_OPERATOR() {
  EXPECT_COMPARE "TRUE_OPERATOR" "$1" $2
}

function EXPECT_UNARY_OPERATOR() {
  EXPECT_COMPARE "UNARY_OPERATOR" $1 $2 $3
}

function EXPECT_BINARY_OPERATOR() {
  EXPECT_COMPARE "BINARY_OPERATOR" $1 $2 $3 $4
}

function EXPECT_EQ() {
  EXPECT_BINARY_OPERATOR $1 -eq $2 $3
}

function EXPECT_NE() {
  EXPECT_BINARY_OPERATOR $1 -ne $2 $3
}

function EXPECT_LT() {
  EXPECT_BINARY_OPERATOR $1 -lt $2 $3
}

function EXPECT_GT() {
  EXPECT_BINARY_OPERATOR $1 -gt $2 $3
}

function EXPECT_LE() {
  EXPECT_BINARY_OPERATOR $1 -le $2 $3
}

function EXPECT_GE() {
  EXPECT_BINARY_OPERATOR $1 -ge $2 $3
}

function EXPECT_STR_EQ() {
  EXPECT_BINARY_OPERATOR $1 = $2 $3
}

function EXPECT_STR_ISNULL() {
  EXPECT_UNARY_OPERATOR -z $1
}

function EXPECT_STR_ISNONNIL() {
  EXPECT_UNARY_OPERATOR -n $1
}

function EXPECT_TRUE() {
  EXPECT_TRUE_OPERATOR "$1" $2
}

function FAILURE() {
  LOGI run $func FAILURE
  errno=0
}

function SUCCESS() {
  LOGI run $func SUCCESS
  errno=0
}

function SUCCESS_COUNT() {
  ALL_SUCCESS_CASES[$suc_count]=$func
  ((suc_count+=1))
}

function FAILURE_COUNT() {
  ALL_FAILURE_CASES[$fail_count]=$func
  ((fail_count+=1))
}


function RESULT() {
  if [ $errno -eq 0 ] ; then
    SUCCESS
    SUCCESS_COUNT
  else
    FAILURE
    FAILURE_COUNT
  fi
}

function RUN() {
  local startTime="`date +%s.%N`"

  $1

  local endTime="`date +%s.%N`"
  local startSec="`echo $startTime | awk -F'.' '{print $1}'`"
  local startMSec="`echo $startTime | awk -F'.' '{print $2}' | sed 's/^0\(.*\)/\1/'`"
  local endSec="`echo $endTime | awk -F'.' '{print $1}'`"
  local endMSec="`echo $endTime | awk -F'.' '{print $2}'| sed 's/^0\(.*\)/\1/'`"

  local costSec=$((($endSec-$startSec)*1000))
  local costMsec=$((($endMSec-$startMSec)/1000000))
  local costTime=$(($costSec+$costMsec))

  LOGI run $1 \($costTime ms total\)
}


