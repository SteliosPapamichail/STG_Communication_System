//
// Created by fresh on 2-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
#define STG_COMMUNICATION_SYSTEM_GROUND_STATION_H

#include <mpi/mpi.h>

void add_parent_gs(int parent_rank);

void add_neighbor_gs(int child_rank);

void add_gs_coords(const float *coords);

float *get_gs_coords();

void set_st_leader(int leader);

int get_st_leader();

#endif //STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
