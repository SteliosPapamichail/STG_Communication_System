//
// Created by fresh on 1-5-24.
//

#include "satellite.h"

#include <stdio.h>
#include <math.h>

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
    int num_replies = 0;
    int flag;
    int left_neighbor = get_left_ring_neighbor(rank, size);
    int right_neighbor = get_right_ring_neighbor(rank, size);
    int leader_rank;
    int leader_declared = 0;

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
                MPI_Recv(&received_data, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm, &status);

                //printf("%d got PROBE FROM LEFT NEIGHBOR %d\n", rank, left_neighbor);

                if (received_data.rank == rank && !leader_declared) {
                    // end done to coordinator with our rank
                    //printf("%d is ST LEADER\n", rank);
                    // send TERMINATE around the ring
                    leader_declared = 1;
                    MPI_Send(&rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE, group_comm);
                } else if (received_data.rank > rank && received_data.hop < pow(2, received_data.phase)) {
                    received_data.hop += 1;
                    //printf("%d from left neighbor %d sending probe to right neighbor %d\n", rank, left_neighbor, right_neighbor);
                    MPI_Send(&received_data, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                } else if (received_data.rank > rank && received_data.hop >= pow(2, received_data.phase)) {
                    //printf("%d from left neighbor %d sending reply to left neighbor\n", rank, left_neighbor);
                    st_lelect_reply lelect_reply;
                    lelect_reply.rank = received_data.rank;
                    lelect_reply.phase = received_data.phase;
                    MPI_Send(&lelect_reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm);
                }
            } else if (status.MPI_TAG == LELECT_PROBE && status.MPI_SOURCE == right_neighbor) {
                st_lelect_probe received_data;
                MPI_Recv(&received_data, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm, &status);

                //printf("%d got PROBE FROM LEFT NEIGHBOR %d\n", rank, left_neighbor);

                if (received_data.rank == rank && !leader_declared) {
                    // end done to coordinator with our rank
                    //printf("%d is ST LEADER\n", rank);
                    leader_declared = 1;
                    MPI_Send(&rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE, group_comm);
                } else if (received_data.rank > rank && received_data.hop < pow(2, received_data.phase)) {
                    //printf("%d from right neighbor %d sending probe to left neighbor %d\n", rank, right_neighbor,left_neighbor);
                    received_data.hop += 1;
                    MPI_Send(&received_data, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                } else if (received_data.rank > rank && received_data.hop >= pow(2, received_data.phase)) {
                    //printf("%d from right neighbor %d sending reply to right neighbor\n", rank, right_neighbor);
                    st_lelect_reply lelect_reply;
                    lelect_reply.rank = received_data.rank;
                    lelect_reply.phase = received_data.phase;
                    MPI_Send(&lelect_reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm);
                }
            } else if (status.MPI_TAG == LELECT_REPLY && status.MPI_SOURCE == left_neighbor) {
                st_lelect_reply reply;
                MPI_Recv(&reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm, &status);

                //printf("%d got REPLY from left neighbor %d\n", rank, left_neighbor);

                if (reply.rank != rank) {
                    // forward reply
                    //printf("forward reply to right neighbor %d\n", right_neighbor);
                    MPI_Send(&reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm);
                } else {
                    // phase k winner
                    if (num_replies > 0) {
                        // got second reply so we win this phase
                        //printf("%d is phase %d winner\n", rank, reply.phase);
                        st_lelect_probe probe;
                        probe.rank = rank;
                        probe.phase = reply.phase + 1;
                        probe.hop = 1;
                        MPI_Send(&probe, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                        MPI_Send(&probe, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                        num_replies = 0; // reset for next phase
                    } else {
                        //printf("got one reply\n");
                        num_replies++;
                    }
                }
            } else if (status.MPI_TAG == LELECT_REPLY && status.MPI_SOURCE == right_neighbor) {
                st_lelect_reply reply;
                MPI_Recv(&reply, 1, reply_datatype, right_neighbor, LELECT_REPLY, group_comm, &status);

                //printf("%d got REPLY from right neighbor %d\n", rank, right_neighbor);

                if (reply.rank != rank) {
                    // forward reply
                    //printf("forward reply to left neighbor %d\n", left_neighbor);
                    MPI_Send(&reply, 1, reply_datatype, left_neighbor, LELECT_REPLY, group_comm);
                } else {
                    if (num_replies == 1) {
                        // got second reply so we win this phase
                        //printf("%d is phase %d winner\n", rank, reply.phase);
                        // phase k winner
                        st_lelect_probe probe;
                        probe.rank = rank;
                        probe.phase = reply.phase + 1;
                        probe.hop = 1;
                        MPI_Send(&probe, 1, probe_datatype, left_neighbor, LELECT_PROBE, group_comm);
                        MPI_Send(&probe, 1, probe_datatype, right_neighbor, LELECT_PROBE, group_comm);
                        num_replies = 0; // reset for next phase
                    } else {
                        //printf("got one reply\n");
                        num_replies++;
                    }
                }
            } else if (status.MPI_TAG == LELECT_TERMINATE && status.MPI_SOURCE == right_neighbor) {
                MPI_Recv(&leader_rank, 1, MPI_INT, right_neighbor, LELECT_TERMINATE, group_comm, &status);
                break;
            }
        }
    } while (1);

    if (leader_rank == rank) {
        // leader payload came full-circle/ring, we can notify coordinator
        //printf("%d sending done\n", rank);
        MPI_Send(&leader_rank, 1, MPI_INT, coordinator_rank, LELECT_ST_DONE, MPI_COMM_WORLD);
    } else {
        // else propagate leader rank to neighbor in ring
        //printf("%d sending terminate to %d. SOURCE WAS %d\n", rank, left_neighbor, leader_rank);
        MPI_Send(&leader_rank, 1, MPI_INT, left_neighbor, LELECT_TERMINATE, group_comm);
    }
}
