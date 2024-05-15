//
// Created by fresh on 2-5-24.
//

#include <stdio.h>
#include <malloc.h>
#include "ground_station.h"

#include <string.h>

#include "../common/event_payloads.h"
#include "../common/utils.h"
#include "../common/mpi_datatypes.h"

int parent_gs = -1; // -1 denotes the rootn
int *neighbor_gs = NULL;
int num_of_neighbors = 0;
float station_coordinates[3];
int st_leader_rank;
int num_of_status_checks_performed = 0;

extern inline double calc_distance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2);

void free_neighbor_gs() {
    if (neighbor_gs != NULL) {
        free(neighbor_gs);
        neighbor_gs = NULL;
    }
}

int get_status_checks_count() {
    return num_of_status_checks_performed;
}

void increment_status_checks_count() {
    num_of_status_checks_performed++;
}

void add_gs_coords(const float *coords) {
    for (int i = 0; i < 3; i++) station_coordinates[i] = coords[i];
}

float *get_gs_coords() {
    return station_coordinates;
}

void add_parent_gs(const int parent_rank) {
    parent_gs = parent_rank;
}

void add_neighbor_gs(const int child_rank) {
    neighbor_gs = realloc(neighbor_gs, (num_of_neighbors + 1) * sizeof(int));
    neighbor_gs[num_of_neighbors++] = child_rank;
}

void set_st_leader_gs(const int leader) {
    st_leader_rank = leader;
}

int get_st_leader_gs() {
    return st_leader_rank;
}

int get_neighbor_count() {
    return num_of_neighbors;
}

void perform_gs_leader_election(int coordinator_rank, int rank, MPI_Comm comm) {
    MPI_Status status;
    int leader_rank = -1;
    int received_count = 0;
    int neighbor_received_from[num_of_neighbors];
    for (int i = 0; i < num_of_neighbors; i++) neighbor_received_from[i] = 0;
    int remaining_neighbor = -1;

    printf("Process %d: Starting leader election\n", rank);

    // Determine if the process is a leaf node
    const int is_leaf = (num_of_neighbors == 1);
    printf("Process %d: Is leaf? %d\n", rank, is_leaf);

    // Send initial <ELECT> messages only if the process is a leaf node
    if (is_leaf) {
        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[0], ELECT, comm);
        printf("Leaf Process %d: Sent ELECT to %d\n", rank, neighbor_gs[0]);
    }

    do {
        int sender_rank;
        while (received_count < num_of_neighbors - 1) {
            MPI_Recv(&sender_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

            if (status.MPI_TAG == ELECT) {
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (neighbor_gs[i] == status.MPI_SOURCE) {
                        printf("setting received from %d\n", neighbor_gs[i]);
                        neighbor_received_from[i] = 1;
                    }
                }
                received_count++;
                printf("Process %d got ELECT from %d (replies = %d)\n", rank, status.MPI_SOURCE, received_count);
                if (received_count == num_of_neighbors - 1) break;
            }

            if (status.MPI_TAG == TERMINATE_LELECT_GS) {
                // leader has been found, terminate and notify all neighbors by sending leader rank
                leader_rank = sender_rank;
                printf("Process %d got TERMINATE from %d with leader %d\n", rank, status.MPI_SOURCE, sender_rank);
                for (int i = 0; i < num_of_neighbors; i++) {
                    printf("Process %d propagating leader to process %d\n", rank, neighbor_gs[i]);
                    MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[i], TERMINATE_LELECT_GS, comm);
                }
                break;
            }
        }

        if (leader_rank != -1) break; // got terminate while still sending elects, terminate early

        printf("iterating for remaining neighbor\n");
        // we have received replies from all neighbors but one
        for (int i = 0; i < num_of_neighbors; i++) {
            printf("considering %d with found %d\n", neighbor_gs[i], neighbor_received_from[i]);
            if (!neighbor_received_from[i]) {
                printf("remaining neighbor is %d\n", neighbor_gs[i]);
                remaining_neighbor = neighbor_gs[i];
                break;
            }
        }

        if (remaining_neighbor != -1 && !is_leaf) {
            // if we are a leaf, don't resend
            printf("Process %d has received all replies but one (from %d)...\n", rank, remaining_neighbor);
            int flag;
            // probe to see if the remaining neighbor is sending an ELECT before we send it one
            MPI_Iprobe(remaining_neighbor, ELECT, comm, &flag, &status);

            if (flag) {
                MPI_Recv(&sender_rank, 1, MPI_INT, remaining_neighbor, ELECT, comm, &status);

                // we got our last elect response, so we are the leader
                printf("Process %d is leader because it received ELECT from remaining neighbor\n", rank);
                // note: could update count and received neighbors but no need
                leader_rank = rank;
                // send out our rank to all neighbors
                for (int j = 0; j < num_of_neighbors; j++) {
                    printf("Process %d propagating winner (us) to %d\n", rank, neighbor_gs[j]);
                    MPI_Send(&rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
                }
                break;
            } else {
                // else we send to the last neighbor
                printf("Process %d, no incoming last elect from probe, sending elect to last neighbor\n", rank);
                MPI_Send(&rank, 1, MPI_INT, remaining_neighbor, ELECT, comm);
            }
        }

        // have sent to all neighbors, wait for outcome
        printf("Process %d has sent all elects, now waiting for final event\n", rank);
        MPI_Recv(&sender_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

        if (status.MPI_TAG == ELECT && status.MPI_SOURCE == remaining_neighbor) {
            // probe for terminate in case someone else was elected after the elect we received was sent
            int flag = 0;
            MPI_Iprobe(MPI_ANY_SOURCE, TERMINATE_LELECT_GS, comm, &flag, &status);

            if (flag) {
                // another process was declared leader faster, abort
                MPI_Recv(&leader_rank, 1, MPI_INT, MPI_ANY_SOURCE, TERMINATE_LELECT_GS, comm, &status);
                printf("Process %d got terminate from %d because another process won first!\n", rank,
                       status.MPI_SOURCE);
                for (int j = 0; j < num_of_neighbors; j++) {
                    printf("Process %d sending terminate to %d\n", rank, neighbor_gs[j]);
                    MPI_Send(&sender_rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
                }
                break;
            }


            // means we got our last elect but after we sent out an elect to the remaining neighbor
            // incoming elect right after we sent elect the same way, so contest
            if (rank > sender_rank) {
                printf("Process %d got elect on same edge from %d and won\n", rank, sender_rank);
                // we win the fight and are the leader
                leader_rank = rank;
                for (int j = 0; j < num_of_neighbors; j++) {
                    printf("Process %d propagating ourself (winner) to %d\n", rank, neighbor_gs[j]);
                    MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
                }
                break;
            } else {
                // sender has won and is leader,, wait for terminate signal with winner rank
                printf("Process %d got elect on same edge and lost fight. winner was %d\n", rank,
                       status.MPI_SOURCE);
                leader_rank = status.MPI_SOURCE;
                for (int j = 0; j < num_of_neighbors; j++) {
                    printf("Process %d propagating other process/winner to %d\n", rank, neighbor_gs[j]);
                    MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
                }
                break;
            }
        } else if (status.MPI_TAG == TERMINATE_LELECT_GS) {
            // we got terminate signal, notify neighbors
            leader_rank = sender_rank;
            printf("Process %d got terminate from %d with leader being %d!\n", rank, status.MPI_SOURCE, leader_rank);
            for (int j = 0; j < num_of_neighbors; j++) {
                MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
            }
            break;
        }
    } while (1);
    printf("Process %d exited loop with leader being %d\n", rank, leader_rank);
    MPI_Barrier(comm);
    if (rank == leader_rank) {
        printf("Leader sending done to coordinator\n");
        MPI_Send(&rank, 1, MPI_INT, coordinator_rank, LELECT_GS_DONE, MPI_COMM_WORLD);
    }
}

int get_min_dist_gs(const int rank, const status_check data, MPI_Comm group_comm, MPI_Datatype stat_check_datatype) {
    int converged = 0;
    double distance = calc_distance(data.st_coords[0], data.st_coords[1], data.st_coords[2], station_coordinates[0],
                                    station_coordinates[1], station_coordinates[2]);

    struct stat_check_gs stat_check_gs;
    stat_check_gs.gs_rank = rank;
    stat_check_gs.distance = distance;

    // pass distance to neighbors
    for (int i = 0; i < num_of_neighbors; i++) {
        printf("-- GS %d sending find min to neighbor %d\n", rank, neighbor_gs[i]);
        MPI_Send(&stat_check_gs, 1, stat_check_datatype, neighbor_gs[i], FIND_MIN_DIST, group_comm);
    }

    do {
        double updated_distance = distance;

        for (int i = 0; i < num_of_neighbors; i++) {
            printf("waiting for reply... (%d)\n", rank);
            MPI_Recv(&stat_check_gs, 1, stat_check_datatype, neighbor_gs[i], 100, group_comm, MPI_STATUS_IGNORE);
            if (stat_check_gs.distance < updated_distance) {
                updated_distance = stat_check_gs.distance;
            }
        }

        // Check for convergence
        if (updated_distance == distance) {
            converged = 1;
        } else {
            distance = updated_distance;
        }
    } while (!converged);

    return distance;
}

void send_check_count_to_leader(const int rank, sync *data, const int source_rank, MPI_Comm group_comm,
                                MPI_Datatype sync_datatype) {
    for (int i = 0; i < num_of_neighbors; i++) {
        if (neighbor_gs[i] == data->gs_leader && data->gs_leader != source_rank) {
            // direct link to the leader
            MPI_Send(data, 1, sync_datatype, data->gs_leader, SYNC, group_comm);
            printf("gs %d has direct link to leader, sent to leader %d...EXITING\n", rank, data->gs_leader + 5);
            return;
        }
    }

    // no direct link, send out through all edges (too tired to write better code :) )
    for (int i = 0; i < num_of_neighbors; i++) {
        // don't send SYNC back through the same link it came from
        if (neighbor_gs[i] != source_rank) {
            MPI_Send(data, 1, sync_datatype, neighbor_gs[i], SYNC,
                     group_comm);
            printf("[GS %d] sent sync to neighbor %d\n", rank, neighbor_gs[i] + 5);
        }
    }
    printf("%d Exiting function call...\n", rank);
}
