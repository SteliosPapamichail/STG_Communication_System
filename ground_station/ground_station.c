//
// Created by fresh on 2-5-24.
//

#include <stdio.h>
#include <malloc.h>
#include "ground_station.h"
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
        neighbor_gs = NULL; // Optional: Reset the pointer to NULL to avoid dangling pointer
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
    int sent_elect_to = -1;
    int received_elect_from = -1;
    int is_leader = 0;

    // Determine if the process is a leaf node
    const int is_leaf = (num_of_neighbors == 1);

    // Send initial <elect> messages only if the process is a leaf node
    if (is_leaf) {
        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[0], ELECT, comm);
        sent_elect_to = neighbor_gs[0];
    }

    // Receive and process messages from neighbors
    int received_count = 0;
    while (1) {
        int sender_rank;
        MPI_Recv(&sender_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

        if (status.MPI_TAG == ELECT) {
            received_count++;
            received_elect_from = status.MPI_SOURCE;

            if (received_elect_from != -1 && received_elect_from == sent_elect_to) {
                // compare ranks
                if (rank >= received_elect_from) {
                    // declare current process as leader
                    // printf("process %d had higher rank than %d, is leader\n", rank + group_size,
                    //        received_elect_from + group_size);
                    for (int i = 0; i < num_of_neighbors; i++) {
                        //printf("is sending terminate to neighbor %d\n", neighbor_gs[i] + group_size);
                        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[i], TERMINATE, comm);
                    }
                    is_leader = 1;
                    break;
                } else {
                    // sender is the leader
                    MPI_Send(&rank, 1, MPI_INT, received_elect_from, GS_LEADER, comm);
                }
            }

            if (received_count == num_of_neighbors - 1) {
                // Send <elect> message to the remaining neighbor
                int remaining_neighbor = -1;
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (neighbor_gs[i] != status.MPI_SOURCE) {
                        remaining_neighbor = neighbor_gs[i];
                        break;
                    }
                }
                if (remaining_neighbor != -1) {
                    MPI_Send(&rank, 1, MPI_INT, remaining_neighbor, ELECT, comm);
                    sent_elect_to = remaining_neighbor;
                }
            }

            // Check if all <elect> messages have been received
            if (received_count == num_of_neighbors) {
                //printf("Process %d elected itself as the leader.\n", rank + group_size);
                for (int i = 0; i < num_of_neighbors; i++) {
                    MPI_Send((void *) 0, 0, MPI_INT, neighbor_gs[i], TERMINATE, comm);
                }
                break;
            }
        } else if (status.MPI_TAG == TERMINATE) {
            //printf("%d got terminate from %d\n", rank + group_size, status.MPI_SOURCE + group_size);
            //printf("has %d neighbors\n", num_of_neighbors);
            if (num_of_neighbors > 1) {
                // Forward TERMINATE message to all neighbors except the sender
                for (int i = 0; i < num_of_neighbors; i++) {
                    if (neighbor_gs[i] != status.MPI_SOURCE) {
                        MPI_Send(&rank, 1, MPI_INT, neighbor_gs[i], TERMINATE, comm);
                    }
                }
            }
            break;
        } else if (status.MPI_TAG == GS_LEADER) {
            // current process has been declared as leader after a rank comparison
            //printf("Process %d was elected as the leader due to rank comparison.\n", rank + group_size);
            for (int i = 0; i < num_of_neighbors; i++) {
                MPI_Send((void *) 0, 0, MPI_INT, neighbor_gs[i], TERMINATE, comm);
            }
            is_leader = 1;
            break;
        }
    }

    MPI_Barrier(comm);
    //printf("GS %d terminated\n", rank + group_size);
    if (is_leader) {
        //printf("GS LEADER %d sending done!\n", rank + group_size);
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
