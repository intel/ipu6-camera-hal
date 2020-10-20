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

ALL_SCRIPT_CASES=()
ALL_SCRIPT_CASES_NUM=0

ALL_SUCCESS_CASES=()
ALL_FAILURE_CASES=()

API_TEST_BIN=icamsrc_test
ALL_API_CASES=()
ALL_API_CASES_NUM=0

CASE_SCRIPT_PREFIX=Script
CASE_API_PREFIX=Api

function CHECK_ENV() {
  if [ -z $2 ]; then
    LOGE "failed to execute!please set env value:$1 first!"
    return 1
  fi
  return 0
}

function DO_EXECUTE() {
  LOGR -------- $1
  is_script=`echo $1 | grep "^$CASE_SCRIPT_PREFIX"`
  if [ ! -z $is_script ] ; then
    DO_SCRIPT_EXECUTE $1
  else
    DO_API_EXECUTE $1
  fi
}

function DO_SCRIPT_EXECUTE() {
  local startTime="`date +%s.%N`"
  local execute=`echo $1 | sed 's/.*\.//g'`

  $execute

  local endTime="`date +%s.%N`"
  local startSec="`echo $startTime | awk -F'.' '{print $1}'`"
  local startMSec="`echo $startTime | awk -F'.' '{print $2}' | sed 's/^0*\(.*\)/\1/'`"
  local endSec="`echo $endTime | awk -F'.' '{print $1}'`"
  local endMSec="`echo $endTime | awk -F'.' '{print $2}' | sed 's/^0*\(.*\)/\1/'`"

  local costSec=$((($endSec-$startSec)*1000))
  local costMsec=$((($endMSec-$startMSec)/1000000))
  local costTime=$(($costSec+$costMsec))

  LOGI run $1 \($costTime ms total\)
}

function DO_API_EXECUTE() {
  local execute=`echo $1 | sed 's/.*\.//g'`
  ./$API_TEST_BIN --gtest_filter=CameraSrcTest.$execute
  LOGI run API $1
}

function EXECUTE_ALL() {
  CHECK_ENV cameraInput $cameraInput
  for((i=0;i<${#ALL_SCRIPT_CASES[@]};i++)) ; do
    DO_EXECUTE ${ALL_SCRIPT_CASES[$i]}
  done

  for((i=0;i<${#ALL_API_CASES[@]};i++)) ; do
    DO_EXECUTE ${ALL_API_CASES[$i]}
  done

  LOGN;LOGN
  LOGS Total cases $((ALL_SCRIPT_CASES_NUM+ALL_API_CASES_NUM))
  LOGS Success cases $suc_count
  LOGS Success cases list:
  LIST_ALL_SUCCESS
  LOGN
  LOGS Failure cases $fail_count
  LOGS Failure cases list:
  LIST_ALL_FAILURE
}

function ADD_ALL_SCRIPT_CASES() {
  for f in `ls cases/*.sh` ; do
    source $f
  done
}

function LIST_ALL_CASES() {
  LIST_ALL_SCRIPT_CASES
  LIST_ALL_API_CASES

  LOGI Total Test Cases: $(($ALL_SCRIPT_CASES_NUM+$ALL_API_CASES_NUM))
}

function LIST_ALL_SCRIPT_CASES() {
  echo Script test cases:
  for((i=0;i<$ALL_SCRIPT_CASES_NUM;i++)) ; do
    echo  "  " $((i+1))\) ${ALL_SCRIPT_CASES[$i]}
  done
}

function ADD_ALL_API_CASES() {
  if [ -x $API_TEST_BIN ] ; then
    API_CASES_NUM=0
    for c in `./$API_TEST_BIN --gtest_list_tests | grep -v "[ERR:|DBG:]\|CameraSrcTest"`; do
      ALL_API_CASES[$ALL_API_CASES_NUM]=$CASE_API_PREFIX.$c
      ALL_API_CASES_NUM=$(($ALL_API_CASES_NUM+1))
    done

  else
    API_CASES=
    API_CASES_NUM=0
  fi
}

function LIST_ALL_API_CASES() {
  echo API based test cases:
  for((i=0;i<$ALL_API_CASES_NUM;i++)) ; do
    echo  "  " $((i+$ALL_SCRIPT_CASES_NUM+1))\) ${ALL_API_CASES[$i]}
  done
  LOGN
}

function LIST_ALL_SUCCESS() {
  for((m=0;m<${#ALL_SUCCESS_CASES[@]};m++)) ; do
      echo ${ALL_SUCCESS_CASES[$m]}
  done
}

function LIST_ALL_FAILURE() {
  for((n=0;n<${#ALL_FAILURE_CASES[@]};n++)) ; do
      echo ${ALL_FAILURE_CASES[$n]}
  done
}

function IS_DIGITAL() {
  if [[ $1 != *[!0-9]* ]]; then
    return 0
  else
    return 1
  fi
}

function EXECUTE() {
  CHECK_ENV cameraInput $cameraInput

  local cases=$#
  local cases_found=0

  LOGI Total cases $cases

  for((i=0;i<$cases;i++)) ; do
    run_case=
    IS_DIGITAL $1
    is_digital=$?
    if [ $is_digital -eq 0 ] ; then
      if [ $(($ALL_SCRIPT_CASES_NUM+$ALL_API_CASES_NUM)) -ge $1 ] ; then
        if [ $ALL_SCRIPT_CASES_NUM -ge $1 ] ; then
          run_case=${ALL_SCRIPT_CASES[$(($1-1))]}
        else
          run_case=${ALL_API_CASES[$(($1-$ALL_SCRIPT_CASES_NUM-1))]}
        fi
        cases_found=1
      else
          echo $1 is large than cases number\($(($ALL_SCRIPT_CASES_NUM+$ALL_API_CASES_NUM))\).
          run_case="index $1"
          cases_found=0
      fi
    else
      for((j=0;j<${#ALL_SCRIPT_CASES[@]};j++)) ; do
        if [ "${ALL_SCRIPT_CASES[$j]}" = $1 ] ; then
          run_case=$1
          cases_found=1
          break
        fi
      done
    fi

    if [ $cases_found -eq 1 ] ; then
      DO_EXECUTE $run_case
    else
      LOGE cases $run_case is not found
    fi

    shift 1
    cases_found=0
  done
}

function EXECUTE_PATTERN() {
  CHECK_ENV cameraInput $cameraInput

  local pattern=$1
  local CASE_NUM=0
  local result=0

  if [ "$pattern" = "contain" ] ; then
    option=
  elif [ "$pattern" = "except" ] ; then
    option="-v"
  fi

  shift 1

  for((j=0;j<${ALL_SCRIPT_CASES_NUM};j++)) ; do
    for n in $@; do
      echo ${ALL_SCRIPT_CASES[$j]} | grep $option $n >/dev/null
      result=$?
      if [ "$pattern" = "contain" ] ; then
        if [ $result -eq 0 ]; then
              break
        fi
      elif [ "$pattern" = "except" ] ; then
        if [ $result -ne 0 ]; then
          break
        fi
      fi
    done

    if [ $result -eq 0 ] ; then
      DO_EXECUTE ${ALL_SCRIPT_CASES[$j]}
      CASE_NUM=$((CASE_NUM+1))
    fi
  done

  for((j=0;j<${ALL_API_CASES_NUM};j++)) ; do
    for m in $@; do
      echo ${ALL_API_CASES[$j]} | grep $option $m >/dev/null
      result=$?
      if [ "$pattern" = "contain" ] ; then
        if [ $result -eq 0 ]; then
            break
        fi
      elif [ "$pattern" = "except" ] ; then
        if [ $result -ne 0 ]; then
          break
        fi
      fi
    done

    if [ $result -eq 0 ] ; then
      DO_EXECUTE ${ALL_API_CASES[$j]}
      CASE_NUM=$((CASE_NUM+1))
    fi
  done


  LOGN;LOGN
  LOGS Total cases $CASE_NUM
  LOGS Success cases $suc_count
  LOGS Success cases list:
  LIST_ALL_SUCCESS
  LOGN
  LOGS Failure cases $fail_count
  LOGS Failure cases list:
  LIST_ALL_FAILURE
}

function REGISTER_CASES() {
  local cases=$#

  for((i=0;i<$cases;i++)) ; do
    ALL_SCRIPT_CASES[$ALL_SCRIPT_CASES_NUM]=$CASE_SCRIPT_PREFIX.$1
    ALL_SCRIPT_CASES_NUM=$((ALL_SCRIPT_CASES_NUM+1))
    shift 1
  done
}

function TEST_ALL_CASES() {
  LIST_ALL_DEVICE
  read -p "Please select one:" dev_idx
  ret=`echo $(GET_SELECTED_DEVICE $dev_idx)`
  InputDevice=`echo $ret | awk -F ' ' '{print $2}'`
  export cameraInput=$InputDevice

  #select pattern
  echo -e "1.contain 3A"
  echo -e "2.except UYVY BGRx RGBx BGR RGB16 kpi interlace"
  read -p "Please select the required pattern(rules):" pt

  case $pt in
    1)
      pattern="contain 3A"
      EXECUTE_PATTERN $pattern
      ;;
    2)
      pattern="except UYVY BGRx RGBx BGR RGB16 interlace kpi"
      EXECUTE_PATTERN $pattern
      ;;
    *)
      LOGI "We don't have cases for this device yet."
      ;;
  esac
}
