//
// Created by fresh on 6-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_COORDINATOR_H
#define STG_COMMUNICATION_SYSTEM_COORDINATOR_H

#include "../common/event_payloads.h"

void set_st_leader_coordinator(int rank);

int get_st_leaader_coordinator();

void set_gs_leader_coordinator(int rank);

int get_gs_leader_coordinator();

gs_connect parse_connect_event(const char *line);

st_add_status parse_add_status_event(const char *line);

st_add_coords parse_st_add_coords_event(const char *line);

gs_add_coords parse_gs_add_coords_event(const char *line);

st_add_metric parse_st_add_metric_event(const char *line);

status_check parse_status_check(const char *line);

avg_earth_temp_request parse_avg_earth_temp(const char *line);

#endif //STG_COMMUNICATION_SYSTEM_COORDINATOR_H
