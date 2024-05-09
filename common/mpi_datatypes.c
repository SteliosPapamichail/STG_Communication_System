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

    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype add_status_event_datatype() {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 1};
    MPI_Aint displacements[2];

    MPI_Get_address(&((st_add_status *) 0)->st_rank, &displacements[0]);
    MPI_Get_address(&((st_add_status *) 0)->status, &displacements[1]);

    MPI_Aint base_address;
    MPI_Get_address(NULL, &base_address);
    for (int i = 1; i < 2; i++) {
        displacements[i] -= displacements[0]; // Calculate displacements relative to base
    }

    MPI_Datatype types[2] = {MPI_INT, MPI_FLOAT};
    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype add_st_coords_event_datatype(const MPI_Datatype coords_datatype) {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 3};
    MPI_Aint displacements[2];

    MPI_Get_address(&((st_add_coords *) 0)->st_rank, &displacements[0]);
    MPI_Get_address(&((st_add_coords *) 0)->coords, &displacements[1]);

    MPI_Aint base_address;
    MPI_Get_address(NULL, &base_address);
    for (int i = 1; i < 2; i++) {
        displacements[i] -= displacements[0]; // Calculate displacements relative to base
    }

    // For the coords array, we specify MPI_FLOAT as the datatype for each element
    MPI_Datatype types[2] = {MPI_INT, coords_datatype};

    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        printf("Failed to create MPI_Datatype for custom event!");
    }

    return datatype;
}

MPI_Datatype coordinates_datatype() {
    MPI_Datatype coord_type;
    MPI_Type_contiguous(3, MPI_FLOAT, &coord_type);
    return coord_type;
}
