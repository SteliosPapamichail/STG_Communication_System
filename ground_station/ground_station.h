//
// Created by fresh on 2-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
#define STG_COMMUNICATION_SYSTEM_GROUND_STATION_H

#include <mpi/mpi.h>

void add_parent_gs(int parent_rank);
void add_neighbor_gs(int child_rank);

#endif //STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
