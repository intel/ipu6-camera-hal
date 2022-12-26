#!/usr/bin/env python3

#
#  Copyright (C) 2017 Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import sys
import os.path
import xml.etree.ElementTree as Tree
from mako.template import Template


class ControlData:
    def __init__(self, base_name):
        self.base_name = base_name
        self.pal_uuid = self._get_pal_uuid()
        self.pal_struct = self._get_pal_data_struct()
        self.hal_uuid = self._get_hal_uuid()
        self.hal_struct = self._get_hal_data_struct()
        self.tag_name = self._get_metadata_tag()

    def _get_hal_uuid(self):
        _hal_prefix = 'camera_control_isp_ctrl_id_'
        return _hal_prefix + self.base_name

    def _get_hal_data_struct(self):
        _hal_prefix = 'camera_control_isp_'
        _hal_suffix = '_t'
        return _hal_prefix + self.base_name + _hal_suffix

    def _get_pal_uuid(self):
        _pal_prefix = 'ia_pal_uuid_algo_'
        return _pal_prefix + self.base_name

    def _get_pal_data_struct(self):
        _pal_prefix = 'ia_pal_algo_'
        _pal_suffix = '_t'
        return _pal_prefix + self.base_name + _pal_suffix

    def _get_metadata_tag(self):
        _pal_prefix = 'INTEL_CONTROL_ISP_'
        return _pal_prefix + self.base_name.upper()


def get_isp_control_list(xml_file):
    def append_namespace(tag):
        namespace = '{http://schemas.android.com/service/camera/metadata/}'
        return namespace + tag

    root = Tree.parse(xml_file).getroot()
    _controls = []

    for ns in root.findall(append_namespace('namespace')):
        if ns.get('name') != 'intel':
            continue

        for section in ns.iter(append_namespace('section')):
            if section.get('name') != 'control_isp':
                continue

            controls = section.find(append_namespace('controls'))
            entries = controls.findall(append_namespace('entry'))
            for entry in entries:
                if entry.get('name') == 'enabledCtrlIds':
                    continue
                _controls.append(ControlData(entry.get('name')))

    return _controls


def get_api_definitions(pal_header, controls):
    end_str = '};\n'
    with open(pal_header, 'rt', encoding='utf-8') as f:
        uuid_list = list()
        struct_list = list()

        for item in controls:
            f.seek(0)
            struct_name = 'struct ' + item.hal_struct + ' IS INVALID'
            struct_definition = 'struct ' + item.hal_struct + ' IS INVALID\n'
            uuid_name = '    ' + item.hal_uuid + ' = INVALID UUID,'
            is_found = False

            for line in f:
                if (item.pal_uuid + ' = ') in line:
                    uuid_name = line.replace(item.pal_uuid, item.hal_uuid).replace('\n', '')

                if ('struct ' + item.pal_struct) in line:
                    is_found = True
                    struct_definition = line.replace(item.pal_struct, item.hal_struct)
                    struct_name = item.hal_struct
                elif is_found:
                    struct_definition += line

                if line == end_str:
                    is_found = False

            uuid_list.append(uuid_name)
            struct_list.append((struct_name, struct_definition))

        return uuid_list, struct_list


def generate_isp_control_header_file():
    control_list = get_isp_control_list(metadata_file)
    hal_uuid, hal_struct = get_api_definitions(pal_header_file, control_list)
    header = Template(filename='IspControl.mako')
    output_header = open('../../include/api/IspControl.h', 'w')
    output_header.write(header.render(uuid_list=hal_uuid, struct_list=hal_struct))
    output_header.close()


def generate_isp_control_utils_file():
    control_list = get_isp_control_list(metadata_file)

    cpp = Template(filename='IspControlInfoMap.mako')
    cpp_output = open('IspControlInfoMap.cpp', 'w')
    cpp_output.write(cpp.render(control_list=control_list))
    cpp_output.close()


pal_header_file = 'ia_pal_types_algo_parameters_autogen.h'
metadata_file = '../metadata/autogen/metadata_properties.xml'

if not os.path.exists(pal_header_file):
    print('Cannot find PAL header file: ' + pal_header_file)
    sys.exit(1)

if not os.path.exists(metadata_file):
    print('Cannot find metadata file: ' + metadata_file)
    sys.exit(1)

generate_isp_control_header_file()
generate_isp_control_utils_file()

os.system('../metadata/autogen/metadata-generate')

