#!/bin/bash
#
#  GStreamer
#  Copyright (C) 2016 Intel Corporation
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

function checkout_capability() {
  inspect_log="`gst-inspect-1.0 icamerasrc`"

  echo $inspect_log | grep YUY2 >/dev/null
  if [ $? -ne 0 ] ; then
    return 1
  fi

  echo $inspect_log | grep "interlace-mode" >/dev/null
  if [ $? -ne 0 ] ; then
    return 1
  fi

  echo $inspect_log | grep "deinterlace-method"  >/dev/null
  if [ $? -ne 0 ] ; then
    return 1
  fi
  return 0
}

function acquire_default_value() {
  default_type=$1

  #locate number of raw of default param
  num_of_raw=`gst-inspect-1.0 icamerasrc | grep -n "$default_type" | awk '{print $1}'`
  num_of_raw=`expr ${num_of_raw%?} + 2`

  #acquire str of this raw,and cut default value
  str_of_line=`gst-inspect-1.0 icamerasrc | sed -n $num_of_raw'p'`
  str_of_line=`echo $str_of_line | tr -d " "`
  def_val=`echo $str_of_line | cut -d ':' -f 3`
  echo $def_val
}

function checkout_default_value() {
  ret_val=`acquire_default_value $1`

  if [ $ret_val -eq 6 ] ; then
    echo "default buffer-count is $ret_val"
  else
    echo " default buffer-count is not 6"
    return 1
  fi

  return 0
}

#Template to check attribute of icamerasrc
#
# template_inspect_attribute()
#
#This template checks:
#1.capability
#2.default value of properties
function template_inspect_attribute() {
  local func=${FUNCNAME[1]}
  execute=$1
  shift 1
  $execute $*

  result=$?

  EXPECT_EQ $result 0

  RESULT
}

#Gstreamer pipeline template to dump Progressive/Interlace image
#
# template_dump_image_progressive_interlace()
#
#Param 1: format
#Param 2: width
#Param 3: height
function template_dump_image_progressive_interlace() {
  local func=${FUNCNAME[1]}
  local FORMAT=$1
  local WIDTH=$2
  local HEIGHT=$3
  local NUMBUF=10
  local INTERLACE_DEINTERLACE=
  local FIELD=0

  if [[ "$func" =~ "interlace" ]] ; then
    INTERLACE_DEINTERLACE="interlace-mode=alternate deinterlace_method=sw_bob"
    FIELD=7
  fi

  gst-launch-1.0 icamerasrc device-name=$cameraInput num-buffers=$NUMBUF $INTERLACE_DEINTERLACE ! video/x-raw,format=$FORMAT,width=$WIDTH,height=$HEIGHT ! filesink location=$func
  result=$?

  if [[ "$func" =~ "abnormal" ]] ; then
    EXPECT_NE $result 0
    EXPECT_TRUE " ! -f $func"
  else
    image_size=`wc -c $func | awk '{print $1}'`
    buffer_size=`echo $(TOOLS_GET_BUFFER_SIZE --getbuffersize -d $cameraInput -m $FORMAT -w $WIDTH -h $HEIGHT -f $FIELD)`

    EXPECT_EQ $result 0
    EXPECT_TRUE " -f $func "
    EXPECT_EQ $image_size $(($buffer_size*$NUMBUF))
  fi

  RESULT
}

#Gstreamer pipeline template to preview Progressive/interlace
#
# template_preview_progressive_interlace()
#
#Param 1: format
#Param 2: width
#Param 3: height
function template_preview_progressive_interlace() {
  local func=${FUNCNAME[1]}
  local FORMAT=$1
  local WIDTH=$2
  local HEIGHT=$3
  local INPUT_FORMAT=
  local INTERLACE_DEINTERLACE=
  local SINKPLUGIN="vaapipostproc ! vaapisink"
  if [ $FORMAT = "BGR" -o $FORMAT = "RGB16" -o $FORMAT = "NV16" ] ; then
    SINKPLUGIN="videoconvert ! ximagesink"
  fi

  if [[ "$func" =~ "interlace" ]] ; then
    INTERLACE_DEINTERLACE="interlace-mode=alternate deinterlace_method=sw_bob"
  fi

  if [[ "$func" =~ "fakesink" ]] ; then
    SINKPLUGIN="fakesink"
  elif [[ "$func" =~ "xvimagesink" ]] ; then
    SINKPLUGIN="xvimagesink"
  fi

  if [[ "$func" =~ "inputformat" ]] ; then
    INPUT_FORMAT="input-format=$4"
  fi

  DISPLAY=:0 gst-launch-1.0 icamerasrc device-name=$cameraInput printfps=true num-buffers=500 $INPUT_FORMAT $INTERLACE_DEINTERLACE ! video/x-raw,format=$FORMAT,width=$WIDTH, height=$HEIGHT ! $SINKPLUGIN | tee $func.log

  result=$?
  local str=`echo $(grep "Average fps" $func.log) | cut -d ',' -f 3`
  local av_fps=`echo ${str:15}`
  local compare_return=$(awk -v num1=$av_fps -v num2=60 'BEGIN{print(num1>num2)?"0":"1"}')
  EXPECT_EQ $result 0
  EXPECT_EQ $compare_return 0

  RESULT
}

#Gstreamer pipeline template to preview Progressive preview in DMA mode
#
# templat_preview_progressive_dma_buffer()
#
#Param 1: format
#Param 2: width
#Param 3: height
#Param 4: value of 'io-mode' property
function templat_preview_progressive_dma_buffer() {
  local func=${FUNCNAME[1]}
  local FORMAT=$1
  local WIDTH=$2
  local HEIGHT=$3
  local IO_MODE=$4

  DISPLAY=:0 gst-launch-1.0 icamerasrc device-name=$cameraInput num-buffers=100 io-mode=$IO_MODE ! video/x-raw,format=$FORMAT,width=$WIDTH, height=$HEIGHT ! vaapipostproc ! vaapisink

  result=$?

  EXPECT_EQ $result 0
  RESULT
}

#Gstreamer pipeline template to preview KPI Progressive/interlace
#
# template_preview_progressive_interlace_kpi()
#
#Param 1: format
#Param 2: width
#Param 3: height
#Param 4: value of 'buffer-count' property
#Param 5: value of 'io-mode' property
function template_preview_progressive_interlace_kpi() {
  local func=${FUNCNAME[1]}
  local FORMAT=$1
  local WIDTH=$2
  local HEIGHT=$3
  local BUF_COUNT=$4
  local IO_MODE=$5
  local INTERLACE=
  local TILED="true"
  local SINKPLUGIN="vaapipostproc ! vaapisink"

  if [[ "$func" =~ "interlace" ]] ; then
    INTERLACE="interlace-mode=alternate"
  fi

  if [[ "$func" =~ "encode" ]] ; then
    TILED="false"
  fi

  if [[ "$func" =~ "fakesink" ]] ; then
    SINKPLUGIN="fakesink"
  elif [[ "$func" =~ "filesink" ]] ; then
    SINKPLUGIN="yamiencode format=H264 ! filesink location=$func.264"
  fi

  DISPLAY=:0 gst-launch-1.0 icamerasrc device-name=$cameraInput printfps=true buffer-count=$BUF_COUNT num-buffers=500 $INTERLACE io-mode=$IO_MODE ! video/x-raw,format=$FORMAT,width=$WIDTH,height=$HEIGHT,tiled=$TILED ! $SINKPLUGIN | tee $func.log

  result=$?

  local str=`echo $(grep "Average fps" $func.log) | cut -d ',' -f 3`
  local av_fps=`echo ${str:15}`
  local compare_return=$(awk -v num1=$av_fps -v num2=60 'BEGIN{print(num1>num2)?"0":"1"}')
  EXPECT_EQ $result 0
  EXPECT_EQ $compare_return 0

  RESULT
}

#Below arrays and function are used for Dual camera and Virtual Channel cases
DEV_NAME_ARR=()
PARAM_LIST=()
FMT_ARR=()
WIDTH_ARR=()
HEIGHT_ARR=()
function config_stream_parameters() {
  LOGI "Total $1 stream(s)"
  local num_of_stream=$1

  #Save all device names into array
  LOGI "Set environment values of cameraInput1,cameraInput2,..cameraInput4 for each stream"
  DEV_NAME_ARR[0]=$cameraInput1
  DEV_NAME_ARR[1]=$cameraInput2
  DEV_NAME_ARR[2]=$cameraInput3
  DEV_NAME_ARR[3]=$cameraInput4

  #Save all parameters into array
  PARAM_LIST=($@)

  #Check if environment is filled with device name
  #Parse PARAM_LIST, and save formats and resolutions into arrays
  for((i=0;i<$num_of_stream;i++))
  do
          CHECK_ENV "cameraInput"$(($i+1)) ${DEV_NAME_ARR[$i]}
          if [ $? -ne 0 ] ; then
            return 1
          fi
          FMT_ARR[$i]=${PARAM_LIST[$((($i+1)*2-1))]}
          WIDTH_ARR[$i]=`echo ${PARAM_LIST[$((($i+1)*2))]} | cut -d 'x' -f 1`
          HEIGHT_ARR[$i]=`echo ${PARAM_LIST[$((($i+1)*2))]} | cut -d 'x' -f 2`
          LOGI "Configurations for Stream$i:"
          LOGI "Used Sensor:${DEV_NAME_ARR[$i]} Format:${FMT_ARR[$i]} Width:${WIDTH_ARR[$i]} Height:${HEIGHT_ARR[$i]}"
  done
}

#Gstreamer pipeline template for multiple streams, including Dual Camera
#and Virtual Channel
#
# template_multiple_streams
#
#Param 1: number of streams
#Param 2,4,6,8..: format of each stream
#Param 3,5,7,9..: widthxheight of each stream
function template_multiple_streams() {
  local func=${FUNCNAME[1]}

  config_stream_parameters $*
  if [ $? -ne 0 ] ; then
    exit 1
  fi

  local NUM_STREAM=$1
  local CAP_FILTER=()
  local SINK_PLUGIN=()
  local IMG_FILE=()
  local STREAM=()

  if [[ "$func" =~ "preview" ]] ; then
    is_preview=1
  else
    is_preview=0
  fi

  if [[ "$func" =~ "dual" ]] ; then
    num_vc_stream=0
  else
    num_vc_stream=$NUM_STREAM
  fi

  for ((i=0;i<$NUM_STREAM;i++)) ; do
      if [ $is_preview -eq 1 ] ; then #generate preview pipeline
        NUM_BUF=500
        if [ "${FMT_ARR[$i]}" = "BGR" -o "${FMT_ARR[$i]}" = "RGB16" -o "${FMT_ARR[$i]}" = "NV16" ] ; then
          SINK_PLUGIN[$i]="videoconvert ! ximagesink"
        else
          SINK_PLUGIN[$i]="vaapipostproc ! vaapisink"
        fi
      else #generate image dump pipeline
        NUM_BUF=10
        IMG_FILE[$i]=$func"_Stream_"$i
        SINK_PLUGIN[$i]="filesink location=${IMG_FILE[$i]}"
      fi

      CAPS_FILTER[$i]="video/x-raw,format=${FMT_ARR[$i]},width=${WIDTH_ARR[$i]},height=${HEIGHT_ARR[$i]}"
      STREAM[$i]="icamerasrc device-name=${DEV_NAME_ARR[$i]} num-vc=$num_vc_stream num-buffers=$NUM_BUF ! ${CAPS_FILTER[$i]} ! ${SINK_PLUGIN[$i]}"
  done

  if [ $is_preview -eq 1 ] ; then
    #Pipeline
    DISPLAY=:0 gst-launch-1.0 ${STREAM[0]} ${STREAM[1]} ${STREAM[2]} ${STREAM[3]}
    result=$?
    EXPECT_EQ $result 0
  else
    #Pipeline
    gst-launch-1.0 ${STREAM[0]} ${STREAM[1]} ${STREAM[2]} ${STREAM[3]}
    result=$?
    EXPECT_EQ $result 0
    #check image for each stream
    for ((j=0;j<$NUM_STREAM;j++)) ; do
      EXPECT_TRUE " -f ${IMG_FILE[$j]} "
      image_size=`wc -c ${IMG_FILE[$j]} | awk '{print $1}'`
      buffer_size=`echo $(TOOLS_GET_BUFFER_SIZE --getbuffersize -d ${DEV_NAME_ARR[$j]} -m ${FMT_ARR[$j]} -w ${WIDTH_ARR[$j]} -h ${HEIGHT_ARR[$j]} -f 0)`
      EXPECT_EQ $image_size $(($buffer_size*$NUM_BUF))
    done
  fi

  RESULT
}
