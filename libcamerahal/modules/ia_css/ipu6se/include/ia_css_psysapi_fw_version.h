/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IA_CSS_PSYSAPI_FW_VERSION_H
#define __IA_CSS_PSYSAPI_FW_VERSION_H

/* PSYSAPI FW VERSION is taken from Makefile for FW tests */
#define BXT_FW_RELEASE_VERSION PSYS_FIRMWARE_VERSION

enum ia_css_process_group_protocol_version {
	/**
	 * Legacy protocol
	 */
	IA_CSS_PROCESS_GROUP_PROTOCOL_LEGACY = 0,
	/**
	 * Persistent process group support protocol
	 */
	IA_CSS_PROCESS_GROUP_PROTOCOL_PPG,
	IA_CSS_PROCESS_GROUP_N_PROTOCOLS
};

#endif /* __IA_CSS_PSYSAPI_FW_VERSION_H */
