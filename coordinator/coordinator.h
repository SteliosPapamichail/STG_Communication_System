//
// Created by fresh on 6-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_COORDINATOR_H
#define STG_COMMUNICATION_SYSTEM_COORDINATOR_H

#include "../common/event_payloads.h"

gs_connect parse_connect_event(const char *line);

st_add_status parse_add_status_event(const char *line);

st_add_coords parse_st_add_coords_event(const char* line);

#endif //STG_COMMUNICATION_SYSTEM_COORDINATOR_H
