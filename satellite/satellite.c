//
// Created by fresh on 1-5-24.
//

#include "satellite.h"

#include <math.h>
#include <stdio.h>

#include "../common/constants.h"
#include "../common/event_payloads.h"

float satellite_status;
float satellite_coordinates[3];
int gs_leader_rank;

int get_gs_leader_st() {
    return gs_leader_rank;
}

void set_gs_leader_st(int gs_rank) {
    gs_leader_rank = gs_rank;
}

// right neighbor according to the leader election
int get_right_ring_neighbor(const int rank, const int total_nodes) {
    return (rank - 1 + total_nodes) % total_nodes;
}

// left neighbor according to the leader election
int get_left_ring_neighbor(const int rank, const int total_nodes) {
    return (rank + 1) % total_nodes;
}

void set_st_status(const float status) {
    satellite_status = status;
}

float get_st_status() {
    return satellite_status;
}

void set_st_coords(const float *coords) {
    for (int i = 0; i < 3; i++) satellite_coordinates[i] = coords[i];
}

float *get_st_coords() {
    return satellite_coordinates;
}

void perform_st_leader_election(int coordinator_rank, int rank, const int size, MPI_Comm group_comm,
                                MPI_Datatype probe_datatype,
                                MPI_Datatype reply_datatype) {
    MPI_Status status;
    int reply_right = 0;
    int reply_left = 0;
    int flag;
    int left_neighbor = get_left_ring_neighbor(rank, size);
    int right_neighbor = get_right_ring_neighbor(rank, size);
    int leader_rank = -1;

    st_lelect_probe payload;
    payload.rank = rank;
    payload.phase = 0;
    payload.hop = 1;

    // send initial probe to neighbors
    MPI_Send(&payload, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
    MPI_Send(&payload, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);

    do {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, group_comm, &flag, &status);

        if (flag) {
            if (status.MPI_TAG == LELECT_PROBE && status.MPI_SOURCE == left_neighbor) {
                st_lelect_probe received_data;
                printf("process %d got PROBE from left %d\n", rank, left_neighbor);
                MPI_Recv(&received_data, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm, &status);

                if (leader_rank != -1) continue; // leader has already been elected, so we wait for terminate

                if (received_data.rank == rank) {
                    leader_rank = rank;
                    printf("process %d got equal id in probe, sending TERMINATE to left %d\n", rank, left_neighbor);
                    MPI_Send(&rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE, group_comm);
                } else if (received_data.rank > rank && received_data.hop < pow(2, received_data.phase)) {
                    received_data.hop += 1;
                    printf("process %d got higher rank in PROBE & d < 2^k, sending PROBE to right %d\n", rank,
                           right_neighbor);
                    MPI_Send(&received_data, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                } else if (received_data.rank > rank && received_data.hop >= pow(2, received_data.phase)) {
                    st_lelect_reply lelect_reply;
                    lelect_reply.rank = received_data.rank;
                    lelect_reply.phase = received_data.phase;
                    printf("process %d got higher rank in PROBE & d >= 2^k, sending REPLY to left %d\n", rank,
                           left_neighbor);
                    MPI_Send(&lelect_reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm);
                }
            } else if (status.MPI_TAG == LELECT_PROBE && status.MPI_SOURCE == right_neighbor) {
                st_lelect_probe received_data;
                MPI_Recv(&received_data, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm, &status);
                printf("process %d got PROBE from right %d\n", rank, right_neighbor);

                if (leader_rank != -1) continue; // leader has already been elected, so we wait for terminate

                if (received_data.rank == rank) {
                    leader_rank = rank;
                    printf("process %d got equal id in probe, sending TERMINATE to left %d\n", rank, left_neighbor);
                    MPI_Send(&rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE, group_comm);
                } else if (received_data.rank > rank && received_data.hop < pow(2, received_data.phase)) {
                    received_data.hop += 1;
                    printf("process %d got higher rank in PROBE & d < 2^k, sending PROBE to left %d\n", rank,
                           left_neighbor);
                    MPI_Send(&received_data, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                } else if (received_data.rank > rank && received_data.hop >= pow(2, received_data.phase)) {
                    st_lelect_reply lelect_reply;
                    lelect_reply.rank = received_data.rank;
                    lelect_reply.phase = received_data.phase;
                    printf("process %d got higher rank in PROBE & d < 2^k, sending REPLY to right %d\n", rank,
                           right_neighbor);
                    MPI_Send(&lelect_reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm);
                }
            } else if (status.MPI_TAG == LELECT_REPLY && status.MPI_SOURCE == left_neighbor) {
                st_lelect_reply reply;
                MPI_Recv(&reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm, &status);
                printf("process %d got REPLY from left %d\n", rank, left_neighbor);
                if (leader_rank != -1) continue; // leader has already been elected, so we wait for terminate

                if (reply.rank != rank) {
                    // forward reply
                    printf("process %d had equal id to the reply, forward it to right %d\n", rank, right_neighbor);
                    MPI_Send(&reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm);
                } else {
                    reply_left = 1;
                    // phase k winner
                    printf("process %d had different id to the reply\n", rank);
                    if (reply_right) {
                        // got second reply so we win this phase
                        st_lelect_probe probe;
                        probe.rank = rank;
                        probe.phase = reply.phase + 1;
                        probe.hop = 1;
                        MPI_Send(&probe, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                        MPI_Send(&probe, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                        printf("process %d had both replies and is phase winner, sent PROBE to left %d and right %d\n",
                               rank, left_neighbor, right_neighbor);
                        reply_left = 0;
                        reply_right = 0;
                    }
                }
            } else if (status.MPI_TAG == LELECT_REPLY && status.MPI_SOURCE == right_neighbor) {
                st_lelect_reply reply;
                MPI_Recv(&reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm, &status);
                printf("process %d got REPLY from right %d\n", rank, right_neighbor);

                if (leader_rank != -1) continue; // leader has already been elected, so we wait for terminate

                if (reply.rank != rank) {
                    // forward reply
                    printf("process %d had equal id to the reply, forward it to left %d\n", rank, left_neighbor);
                    MPI_Send(&reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm);
                } else {
                    reply_right = 1;
                    printf("process %d had different id to the reply\n", rank);
                    if (reply_left) {
                        // got second reply so we win this phase
                        // phase k winner
                        st_lelect_probe probe;
                        probe.rank = rank;
                        probe.phase = reply.phase + 1;
                        probe.hop = 1;
                        MPI_Send(&probe, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                        MPI_Send(&probe, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                        printf("process %d had both replies and is phase winner, sent PROBE to left %d and right %d\n",
                               rank, left_neighbor, right_neighbor);
                        reply_left = 0;
                        reply_right = 0;
                    }
                }
            } else if (status.MPI_TAG == LELECT_TERMINATE && status.MPI_SOURCE == right_neighbor) {
                MPI_Recv(&leader_rank, 1, MPI_INT, right_neighbor, LELECT_TERMINATE, group_comm, &status);
                printf("process %d got terminate from %d\n", rank, right_neighbor);

                if (leader_rank != rank) {
                    MPI_Send(&leader_rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE,
                             group_comm);
                    printf("process %d sent terminate to left %d\n", rank, left_neighbor);
                }
                break;
            }
        }
    } while (1);

    printf("process %d exited election\n", rank);

    if (leader_rank == rank) {
        printf("leader %d\n", leader_rank);
        // leader payload came full-circle/ring, we can notify coordinator
        MPI_Send(&leader_rank, 1, MPI_INT, coordinator_rank, LELECT_ST_DONE, MPI_COMM_WORLD);
    }
}
