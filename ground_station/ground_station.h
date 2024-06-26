//
// Created by fresh on 2-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
#define STG_COMMUNICATION_SYSTEM_GROUND_STATION_H

#include <mpi/mpi.h>

#include "../common/event_payloads.h"
#include "../common/temp_metrics.h"

typedef struct {
    int satellite_rank;
    float status;
} status_info;

void save_status(status_info data);

void destroy_status_list();

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

void calc_dist_and_broadcast(const int rank, int sender, const status_check data, MPI_Comm group_comm,
                             MPI_Datatype stat_check_datatype);

int get_min_dist_gs(int rank, const int group_size, status_check data, MPI_Comm group_comm,
                    MPI_Datatype stat_check_datatype);

void send_check_count_to_leader(int rank, sync *data, int source_rank, MPI_Comm group_comm, MPI_Datatype sync_datatype);

void write_status_file(int rank);

void initiate_print_broadcast(int rank, int source_rank, MPI_Comm comm);

void write_metrics_file(int rank, const metrics_list *list);

void receive_print_done_and_notify(const int coordinator_rank, const int rank,
                                   const int group_size, MPI_Comm comm);

void send_print_done(const int rank, MPI_Comm comm);

#endif //STG_COMMUNICATION_SYSTEM_GROUND_STATION_H
