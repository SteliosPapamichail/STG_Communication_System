//
// Created by fresh on 2-5-24.
//

#include <stdio.h>
#include <malloc.h>
#include "ground_station.h"
#include "../common/event_payloads.h"
#include "../common/utils.h"
#include "../common/mpi_datatypes.h"
#include "../common/debug_utils.h"

int parent_gs = -1; // -1 denotes the root
int *neighbor_gs = NULL;
int num_of_neighbors = 0;
float station_coordinates[3];
int st_leader_rank;
int leader_gs;

// print-related vars
int num_of_status_checks_performed = 0;
status_info *status_list = NULL;

void save_status(const status_info data) {
    num_of_status_checks_performed++;
    status_list = realloc(status_list, num_of_status_checks_performed * sizeof(status_info));
    // handle realloc errors :)
    status_list[num_of_status_checks_performed - 1] = data;
}

void destroy_status_list() {
    free(status_list);
    status_list = NULL;
}

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

    debug_printf("Process %d: Starting leader election\n", rank);

    // Determine if the process is a leaf node
    const int is_leaf = (num_of_neighbors == 1);
    debug_printf("Process %d: Is leaf? %d\n", rank, is_leaf);

    // Send initial <ELECT> messages only if the process is a leaf node
    if (is_leaf) {
        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[0], ELECT, comm);
        debug_printf("Leaf Process %d: Sent ELECT to %d\n", rank, neighbor_gs[0]);
    }

    do {
        int sender_rank;
        while (received_count < num_of_neighbors - 1) {
            MPI_Recv(&sender_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

            if (status.MPI_TAG == ELECT) {
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (neighbor_gs[i] == status.MPI_SOURCE) {
                        neighbor_received_from[i] = 1;
                    }
                }
                received_count++;
                debug_printf("Process %d got ELECT from %d (replies = %d)\n", rank, status.MPI_SOURCE, received_count);
                if (received_count == num_of_neighbors - 1) break;
            }

            if (status.MPI_TAG == TERMINATE_LELECT_GS) {
                // leader has been found, terminate and notify all neighbors by sending leader rank
                leader_rank = sender_rank;
                debug_printf("Process %d got TERMINATE from %d with leader %d\n", rank, status.MPI_SOURCE, sender_rank);
                for (int i = 0; i < num_of_neighbors; i++) {
                    debug_printf("Process %d propagating leader to process %d\n", rank, neighbor_gs[i]);
                    if (neighbor_gs[i] != status.MPI_SOURCE)
                        MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[i],
                                 TERMINATE_LELECT_GS, comm);
                }
                break;
            }
        }

        if (leader_rank != -1) break; // got terminate while still sending elects, terminate early

        debug_printf("iterating for remaining neighbor\n");
        // we have received replies from all neighbors but one
        for (int i = 0; i < num_of_neighbors; i++) {
            debug_printf("considering %d with found %d\n", neighbor_gs[i], neighbor_received_from[i]);
            if (!neighbor_received_from[i]) {
                debug_printf("remaining neighbor is %d\n", neighbor_gs[i]);
                remaining_neighbor = neighbor_gs[i];
                break;
            }
        }

        if (remaining_neighbor != -1 && !is_leaf) {
            // if we are a leaf, don't resend
            debug_printf("Process %d has received all replies but one (from %d)...\n", rank, remaining_neighbor);
            int flag;
            // probe to see if the remaining neighbor is sending an ELECT before we send it one
            MPI_Iprobe(remaining_neighbor, ELECT, comm, &flag, &status);

            if (flag) {
                MPI_Recv(&sender_rank, 1, MPI_INT, remaining_neighbor, ELECT, comm, &status);

                // we got our last elect response, so we are the leader
                debug_printf("Process %d is leader because it received ELECT from remaining neighbor\n", rank);
                // note: could update count and received neighbors but no need
                leader_rank = rank;
                // send out our rank to all neighbors
                for (int j = 0; j < num_of_neighbors; j++) {
                    debug_printf("Process %d propagating winner (us) to %d\n", rank, neighbor_gs[j]);
                    MPI_Send(&rank, 1, MPI_INT, neighbor_gs[j], TERMINATE_LELECT_GS, comm);
                }
                break;
            } else {
                // else we send to the last neighbor
                debug_printf("Process %d, no incoming last elect from probe, sending elect to last neighbor\n", rank);
                MPI_Send(&rank, 1, MPI_INT, remaining_neighbor, ELECT, comm);
            }
        }

        // have sent to all neighbors, wait for outcome
        debug_printf("Process %d has sent all elects, now waiting for final event\n", rank);
        MPI_Recv(&sender_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

        if (status.MPI_TAG == ELECT && status.MPI_SOURCE == remaining_neighbor) {
            // probe for terminate in case someone else was elected after the elect we received was sent
            int flag = 0;
            MPI_Iprobe(MPI_ANY_SOURCE, TERMINATE_LELECT_GS, comm, &flag, &status);

            if (flag) {
                // another process was declared leader faster, abort
                MPI_Recv(&leader_rank, 1, MPI_INT, MPI_ANY_SOURCE, TERMINATE_LELECT_GS, comm, &status);
                debug_printf("Process %d got terminate from %d because another process won first!\n", rank,
                       status.MPI_SOURCE);
                for (int j = 0; j < num_of_neighbors; j++) {
                    debug_printf("Process %d sending terminate to %d\n", rank, neighbor_gs[j]);
                    if (neighbor_gs[j] != status.MPI_SOURCE)
                        MPI_Send(&sender_rank, 1, MPI_INT, neighbor_gs[j],
                                 TERMINATE_LELECT_GS, comm);
                }
                break;
            }


            // means we got our last elect but after we sent out an elect to the remaining neighbor
            // incoming elect right after we sent elect the same way, so contest
            if (rank > sender_rank) {
                debug_printf("Process %d got elect on same edge from %d and won\n", rank, sender_rank);
                // we win the fight and are the leader
                leader_rank = rank;
                for (int j = 0; j < num_of_neighbors; j++) {
                    debug_printf("Process %d propagating ourself (winner) to %d\n", rank, neighbor_gs[j]);
                    if (neighbor_gs[j] != remaining_neighbor)
                        MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j],
                                 TERMINATE_LELECT_GS, comm);
                }
                break;
            } else {
                // sender has won and is leader
                debug_printf("Process %d got elect on same edge and lost fight. winner was %d\n", rank,
                       status.MPI_SOURCE);
                leader_rank = status.MPI_SOURCE;
                for (int j = 0; j < num_of_neighbors; j++) {
                    debug_printf("Process %d propagating other process/winner to %d\n", rank, neighbor_gs[j]);
                    if (neighbor_gs[j] != remaining_neighbor)
                        MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j],
                                 TERMINATE_LELECT_GS, comm);
                }
                break;
            }
        } else if (status.MPI_TAG == TERMINATE_LELECT_GS) {
            // we got terminate signal, notify neighbors
            leader_rank = sender_rank;
            debug_printf("Process %d got terminate from %d with leader being %d!\n", rank, status.MPI_SOURCE, leader_rank);
            for (int j = 0; j < num_of_neighbors; j++) {
                if (neighbor_gs[j] != remaining_neighbor)
                    MPI_Send(&leader_rank, 1, MPI_INT, neighbor_gs[j],
                             TERMINATE_LELECT_GS, comm);
            }
            break;
        }
    } while (1);
    debug_printf("Process %d exited loop with leader being %d\n", rank, leader_rank);
    MPI_Barrier(comm);
    leader_gs = leader_rank;
    if (rank == leader_rank) {
        debug_printf("Leader sending done to coordinator\n");
        MPI_Send(&rank, 1, MPI_INT, coordinator_rank, LELECT_GS_DONE, MPI_COMM_WORLD);
    }
}

void calc_dist_and_broadcast(const int rank, const int sender, const status_check data, MPI_Comm group_comm,
                             MPI_Datatype stat_check_datatype) {
    const double distance = calc_distance(data.st_coords[0], data.st_coords[1], data.st_coords[2],
                                          station_coordinates[0],
                                          station_coordinates[1], station_coordinates[2]);
    MPI_Status status;
    int flag;
    struct stat_check_gs stat_check_gs;
    stat_check_gs.gs_rank = rank;
    stat_check_gs.distance = distance;


    //TODO:SP maybe use a seperate datatype for REPLY

    // pass distance to neighbors
    for (int i = 0; i < num_of_neighbors; i++) {
        if (sender != neighbor_gs[i]) {
            // notify neighbors to calc their distance
            debug_printf("process %d sending find min to neighbor %d\n", rank, neighbor_gs[i]);
            MPI_Send(&data, 1, stat_check_datatype, neighbor_gs[i], FIND_MIN_DIST,
                     group_comm);
        } else {
            // notify sender of our calculation
            MPI_Send(&stat_check_gs, 1, stat_check_datatype, sender, FIND_DIST_REPLY, group_comm);
            debug_printf("process %d notified sender %d of its distance\n", rank, sender);
        }
    }

    do {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, group_comm, &flag, &status);

        if (flag) {
            if (status.MPI_TAG == FIND_DIST_REPLY) {
                MPI_Recv(&stat_check_gs, 1, stat_check_datatype, status.MPI_SOURCE, FIND_DIST_REPLY, group_comm,
                         &status);
                debug_printf("process %d got REPLY from %d via %d\n", rank, stat_check_gs.gs_rank, status.MPI_SOURCE);
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (status.MPI_SOURCE != neighbor_gs[i]) {
                        // forward reply
                        debug_printf("process %d sending REPLY to neighbor %d\n", rank, neighbor_gs[i]);
                        MPI_Send(&stat_check_gs, 1, stat_check_datatype, neighbor_gs[i], FIND_DIST_REPLY, group_comm);
                    }
                }
            } else if (status.MPI_TAG == TERMINATE_FIND_DIST) {
                MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, TERMINATE_FIND_DIST, group_comm, &status);
                debug_printf("process %d got TERMINATE_FIND_DIST from %d\n", rank, status.MPI_SOURCE);
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (status.MPI_SOURCE != neighbor_gs[i]) {
                        // forward reply
                        debug_printf("process %d sending TERMINATE_FIND_DIST to neighbor %d\n", rank, neighbor_gs[i]);
                        MPI_Send((void *) 0, 0, MPI_INT, neighbor_gs[i], TERMINATE_FIND_DIST, group_comm);
                    }
                }
                break;
            }
        }
    } while (1);

    debug_printf("!!! process %d exited calculations\n", rank);
}

int get_min_dist_gs(const int rank, const int group_size, const status_check data, MPI_Comm group_comm,
                    MPI_Datatype stat_check_datatype) {
    int replies = 0;
    int min_gs = rank;
    const double distance = calc_distance(data.st_coords[0], data.st_coords[1], data.st_coords[2],
                                          station_coordinates[0],
                                          station_coordinates[1], station_coordinates[2]);
    MPI_Status status;
    struct stat_check_gs stat_check_gs;

    // pass distance to neighbors
    for (int i = 0; i < num_of_neighbors; i++) {
        debug_printf("process leader %d sending find min to neighbor %d\n", rank, neighbor_gs[i]);
        MPI_Send(&data, 1, stat_check_datatype, neighbor_gs[i], FIND_MIN_DIST, group_comm);
    }

    do {
        double updated_distance = distance;
        debug_printf("leader waiting for reply... (%d)\n", rank);
        MPI_Recv(&stat_check_gs, 1, stat_check_datatype, MPI_ANY_SOURCE, FIND_DIST_REPLY, group_comm, &status);
        debug_printf("process %d got reply from %d via %d with distance %f\n", rank, stat_check_gs.gs_rank, status.MPI_SOURCE,
               stat_check_gs.distance);
        if (stat_check_gs.distance < updated_distance) {
            debug_printf("sender had smaller distance (%f) than leader (%f) and is now min_Gs\n", stat_check_gs.distance,
                   updated_distance);
            updated_distance = stat_check_gs.distance;
            min_gs = status.MPI_SOURCE;
        }
        replies++;

        debug_printf("REPLIES = %d\n", replies);

        // Check for convergence
        if (replies == group_size - 1) {
            for (int i = 0; i < num_of_neighbors; i++) {
                debug_printf("process %d sending TERMINATE_FIND_DIST to %d\n", rank, neighbor_gs[i]);
                MPI_Send((void *) 0, 0, MPI_INT, neighbor_gs[i], TERMINATE_FIND_DIST, group_comm);
            }
            break;
        }
    } while (1);

    debug_printf("=============== found min_gs = %d\n", min_gs);
    return min_gs;
}

void send_check_count_to_leader(const int rank, sync *data, const int source_rank, MPI_Comm group_comm,
                                MPI_Datatype sync_datatype) {
    for (int i = 0; i < num_of_neighbors; i++) {
        if (neighbor_gs[i] == data->gs_leader && data->gs_leader != source_rank) {
            // direct link to the leader
            MPI_Send(data, 1, sync_datatype, data->gs_leader, SYNC, group_comm);
            debug_printf("gs %d has direct link to leader, sent to leader %d...EXITING\n", rank, data->gs_leader + 5);
            return;
        }
    }

    // no direct link, send out through all edges (too tired to write better code :) )
    for (int i = 0; i < num_of_neighbors; i++) {
        // don't send SYNC back through the same link it came from
        if (neighbor_gs[i] != source_rank) {
            MPI_Send(data, 1, sync_datatype, neighbor_gs[i], SYNC,
                     group_comm);
            debug_printf("[GS %d] sent sync to neighbor %d\n", rank, neighbor_gs[i] + 5);
        }
    }
    debug_printf("%d Exiting function call...\n", rank);
}

void write_status_file(const int rank) {
    char filename[256];
    sprintf(filename, "gs_%d_statuses.txt", rank);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create file\n");
        return;
    }

    for (int i = 0; i < num_of_status_checks_performed; i++) {
        fprintf(file, "satellite %d: %.2f\n", status_list[i].satellite_rank, status_list[i].status);
    }

    fclose(file);
}

void write_metrics_file(const int rank, const metrics_list *list) {
    char filename[256];
    sprintf(filename, "gs_leader_%d_metrics.txt", rank);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to create file\n");
        return;
    }
    print_metrics_to_file(file, list);
    fclose(file);
}

void initiate_print_broadcast(const int rank, const int source_rank, MPI_Comm comm) {
    write_status_file(rank); // create leader's status file
    debug_printf("GS %d wrote status file!\n", rank);
    // broadcast print to neighbors
    for (int i = 0; i < num_of_neighbors; i++) {
        if (neighbor_gs[i] != source_rank) {
            MPI_Send((void *) 0, 0, MPI_INT, neighbor_gs[i], PRINT, comm);
            debug_printf("gs %d sent print to %d\n", rank, neighbor_gs[i]);
        }
    }
}

void send_print_done(const int rank, MPI_Comm comm) {
    for (int i = 0; i < num_of_neighbors; i++) {
        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[i], PRINT_DONE, comm);
        debug_printf("gs %d sent print_done to %d\n", rank, neighbor_gs[i]);
    }
}

void receive_print_done_and_notify(const int coordinator_rank, const int rank,
                                   const int group_size, MPI_Comm comm) {
    int count = 0;
    MPI_Status status;
    int sender;
    int done = 0;

    while (!done) {
        MPI_Recv(&sender, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

        if (status.MPI_TAG == PRINT_DONE) {
            debug_printf("Node %d received PRINT_DONE from %d via %d\n", rank, sender, status.MPI_SOURCE);

            if (rank == leader_gs) {
                count++;
                debug_printf("leader incrementing count. Now %d\n", count);

                if (count == group_size - 1) {
                    done = 1;
                    for (int i = 0; i < num_of_neighbors; i++) {
                        MPI_Send(&sender, 1, MPI_INT, neighbor_gs[i], TERMINATE, comm);
                        debug_printf("process %d (leader) got all replies, sent terminate to %d\n", rank, neighbor_gs[i]);
                    }
                }
            } else {
                for (int i = 0; i < num_of_neighbors; i++) {
                    // propagate print done while not resending to sender
                    if (status.MPI_SOURCE != neighbor_gs[i]) {
                        MPI_Send(&sender, 1, MPI_INT, neighbor_gs[i], PRINT_DONE, comm);
                        debug_printf("process %d propagated print_done to %d\n", rank, neighbor_gs[i]);
                    }
                }
            }
        } else if (status.MPI_TAG == TERMINATE) {
            done = 1;
            debug_printf("Node %d received TERMINATE signal from %d\n", rank, status.MPI_SOURCE);
            for (int i = 0; i < num_of_neighbors; i++) {
                if (status.MPI_SOURCE != neighbor_gs[i]) {
                    MPI_Send(&sender, 1, MPI_INT, neighbor_gs[i], TERMINATE, comm);
                    debug_printf("process %d got TERMINATE, sent terminate to %d\n", rank, neighbor_gs[i]);
                }
            }
        }
    }

    MPI_Barrier(comm);
    if (rank == leader_gs) {
        MPI_Send((void *) 0, 0, MPI_INT, coordinator_rank, PRINT_DONE, MPI_COMM_WORLD);
        debug_printf("===> Leader %d has notified coordinator and terminated\n", rank);
    } else {
        debug_printf("===> Node %d has terminated\n", rank);
    }
}
