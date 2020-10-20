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

get_str(){
    ext=$1
    y_time=$2

    for suffix in h cpp cc c java json txt
    do
        if [ "$ext" = $suffix -a ! -z "y_time" ]; then
            str="\ * Copyright (C) $y_time Intel Corporation"
        fi
    done

    for suffix in Makefile py am sh mk README
    do
        if [ "$ext" = $suffix -a ! -z "y_time" ]; then
            str="#  Copyright (C) $y_time Intel Corporation"
        fi
    done

    if [ "$ext" = "xml" -a -n "y_time" ];then
        str="\  Copyright (C) $y_time Intel Corporation"
    fi

    echo "$str"
}

file_path=$1
cr_template=$2
y_time=$3
flag_insert=$4
flag_update=$5

auth=`stat -c "%a" $file_path`
file_name=$(echo ${file_path##*/})

if [ "$file_name" = "Makefile" ]; then
    ext=$file_name
else
    ext=${file_name##*.}
fi

insert_str=$(get_str $ext $y_time)

if [ -e "$file_path" ]; then
    if [ -n "$y_time" ];then
            ##### insert intel copyright into existed copyrights
        if [ "$flag_insert" = "true" ]; then
            line_num=`awk '/Copyright/{print NR}' $file_path`
            last=${line_num:((${#line_num} - 1))}
            insert_line=$(expr $last + 1)
            sed -i "$insert_line i $insert_str" $file_path
            ##### update time for intel copyright
        elif [ "$flag_update" = "true" ]; then
            x=`grep "Copyright" $file_path`
            y=`echo "$x" | grep "Intel" | grep -o "[0-9]\{4\}.*\s" |  cut -d' ' -f 1 | cut -d'-' -f 1`
            z="$y-$y_time"
            if [[ "$ext" = "py" || "$ext" = "sh" || "$ext" = "mk" || "$ext" = "Makefile" || "$ext" = "am" || "$ext" = "README" ]]; then
                sed -i "s/#\s*Copyright.*Corp.*$/#  Copyright (C) $z Intel Corporation/g" $file_path
            elif [ "$ext" != "xml" ]; then
                sed -i "s/ * Copyright.*Corp.*$/ Copyright (C) $z Intel Corporation/g" $file_path
            else
                sed -i "s/\s*Copyright.*Corp.*$/  Copyright (C) $z Intel Corporation/g" $file_path
            fi
            #### add copyright section to file
        else
            if [ -e "$cr_template" ]; then
                if [[ "$ext" = "py" || "$ext" = "sh" ]]; then
                    sed -i "1 r $cr_template" $file_path
                    sed -i '/#\s*GStreamer/a\'"${insert_str#\.\/}"'' $file_path
                else
                    cat $cr_template $file_path > $file_path.tmp
                    line_num=`awk '/GStreamer/{print NR}' $file_path.tmp`
                    insert_line=$(expr ${line_num:0:1} + 1)
                    sed -i "$insert_line i $insert_str" $file_path.tmp
                    mv $file_path.tmp $file_path
                    sudo chmod $auth $file_path
                fi
            else
                echo "No correct copyright template to use."
                exit 1
            fi
       fi
   fi
else
    echo "No file passed to update copyright."
    exit 1
fi
