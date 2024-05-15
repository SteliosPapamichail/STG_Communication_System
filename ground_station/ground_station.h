//
// Created by fresh on 2-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
#define STG_COMMUNICATION_SYSTEM_GROUND_STATION_H

#include <mpi/mpi.h>

#include "../common/event_payloads.h"

void free_neighbor_gs();

int get_status_checks_count();

void increment_status_checks_count();

void add_parent_gs(int parent_rank);

void add_neighbor_gs(int child_rank);

void add_gs_coords(const float *coords);

int get_neighbor_count();

float *get_gs_coords();

void set_st_leader_gs(int leader);

int get_st_leader_gs();

void perform_gs_leader_election(int coordinator_rank, int rank, MPI_Comm comm);

int get_min_dist_gs(int rank, status_check data, MPI_Comm group_comm, MPI_Datatype stat_check_datatype);

void send_check_count_to_leader(int rank, sync *data, int source_rank, MPI_Comm group_comm, MPI_Datatype sync_datatype);

#endif //STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
