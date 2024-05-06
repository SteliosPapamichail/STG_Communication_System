//
// Created by fresh on 2-5-24.
//

#include <stdio.h>
#include "ground_station.h"
#include "../common/event_payloads.h"

//void process_connect_event(MPI_Comm gs_comm, int rank, int neighbor_rank) {
//    // Send CONNECT message to ground station
//    gs_connect connect_message;
//    connect_message.gs_rank = rank;
//    connect_message.neighbor_gs_rank = neighbor_rank;
//
//    MPI_Send(&connect_message, sizeof(ConnectionMessage),
//             MPI_INT, neighbor_rank, 0, gs_comm);
//
//    // Receive message from ground station
//    MPI_Status status;
//    int neighbor_message_type;
//    MPI_Recv(&neighbor_message_type, 1, MPI_INT, neighbor_rank, 0, MPI_COMM_WORLD, &status);
//
//    // Check message type
//    if (neighbor_message_type != ACK) {
//        printf("Error: Expected ACK message from rank %d\n", neighbor_rank);
//        return;
//    }
//
//    // Send ACK message to ground station
//    MPI_Send(&neighbor_message_type, 1, MPI_INT, neighbor_rank, 0, MPI_COMM_WORLD);
//
//    // Receive ACK message from ground station
//    MPI_Recv(&neighbor_message_type, 1, MPI_INT, neighbor_rank, 0, MPI_COMM_WORLD, &status);
//
//    // Check message type
//    if (neighbor_message_type != ACK) {
//        printf("Error: Expected ACK message from rank %d\n", neighbor_rank);
//        return;
//    }
//
//    printf("Coordinator: Connected ground station %d to ground station %d\n", rank, neighbor_rank);
//}