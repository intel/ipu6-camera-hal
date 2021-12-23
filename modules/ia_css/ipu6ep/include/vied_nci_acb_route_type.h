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

#ifndef VIED_NCI_ACB_ROUTE_TYPE_H_
#define VIED_NCI_ACB_ROUTE_TYPE_H_

#include "type_support.h"

typedef enum {
    NCI_ACB_PORT_ISP = 0,
    NCI_ACB_PORT_ACC = 1,
    NCI_ACB_PORT_INVALID = 0xFF
} nci_acb_port_t;

typedef struct {
    /* 0 = ISP, 1 = Acc */
    nci_acb_port_t in_select;
    /* 0 = ISP, 1 = Acc */
    nci_acb_port_t out_select;
    /* When set, Ack will be sent only when Eof arrives */
    uint32_t ignore_line_num;
    /* Fork adapter to enable streaming to both output
     * (next acb out and isp out)
     */
    uint32_t fork_acb_output;
} nci_acb_route_t;

#endif /* VIED_NCI_ACB_ROUTE_TYPE_H_ */
