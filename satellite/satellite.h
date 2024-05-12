//
// Created by fresh on 1-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_SATELLITE_H
#define STG_COMMUNICATION_SYSTEM_SATELLITE_H

#include <mpi/mpi.h>

float get_st_status();

void set_st_status(float status);

void set_st_coords(const float *coords);

float *get_st_coords();

int get_gs_leader_st();

void set_gs_leader_st(int gs_rank);

int get_left_ring_neighbor(int rank, int total_nodes);

int get_right_ring_neighbor(int rank, int total_nodes);

void perform_st_leader_election(int coordinator_rank, int rank, int size, MPI_Comm group_comm,
                                MPI_Datatype probe_datatype, MPI_Datatype reply_datatype);

#endif //STG_COMMUNICATION_SYSTEM_SATELLITE_H
