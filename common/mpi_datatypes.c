//
// Created by fresh on 6-5-24.
//

#include <stdio.h>
#include "mpi_datatypes.h"

MPI_Datatype connect_event_datatype() {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 1}; // Array of lengths for each member
    MPI_Aint displacements[2]; // Array of displacements for each member

    MPI_Get_address(&((gs_connect *) 0)->gs_rank, &displacements[0]);
    MPI_Get_address(&((gs_connect *) 0)->neighbor_gs_rank, &displacements[1]);

    MPI_Aint base_address;
    MPI_Get_address(NULL, &base_address);
    for (int i = 1; i < 2; i++) {
        displacements[i] -= displacements[0]; // Calculate displacements relative to base
    }

    MPI_Datatype types[2] = {MPI_INT, MPI_INT}; // MPI datatypes for members

    int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}