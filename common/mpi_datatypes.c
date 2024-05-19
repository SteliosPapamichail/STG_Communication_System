//
// Created by fresh on 6-5-24.
//

#include <stdio.h>
#include "mpi_datatypes.h"
#include "debug_utils.h"
#include "constants.h"

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
        debug_printf("Failed to create MPI_Datatype for custom event!");
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
        debug_printf("Failed to create MPI_Datatype for custom event!");
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
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }

    return datatype;
}

MPI_Datatype add_gs_coords_event_datatype(MPI_Datatype coords_datatype) {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 3};
    MPI_Aint displacements[2];

    MPI_Get_address(&((gs_add_coords *) 0)->gs_rank, &displacements[0]);
    MPI_Get_address(&((gs_add_coords *) 0)->coords, &displacements[1]);

    MPI_Aint base_address;
    MPI_Get_address(NULL, &base_address);
    for (int i = 1; i < 2; i++) {
        displacements[i] -= displacements[0]; // Calculate displacements relative to base
    }

    // For the coords array, we specify MPI_FLOAT as the datatype for each element
    MPI_Datatype types[2] = {MPI_INT, coords_datatype};

    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }

    return datatype;
}

MPI_Datatype status_check_event_datatype(MPI_Datatype coords_datatype) {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 3};
    MPI_Aint displacements[2];

    MPI_Get_address(&((status_check *) 0)->st_rank, &displacements[0]);
    MPI_Get_address(&((status_check *) 0)->st_coords, &displacements[1]);

    MPI_Aint base_address;
    MPI_Get_address(NULL, &base_address);
    for (int i = 1; i < 2; i++) {
        displacements[i] -= displacements[0]; // Calculate displacements relative to base
    }

    // For the coords array, we specify MPI_FLOAT as the datatype for each element
    MPI_Datatype types[2] = {MPI_INT, coords_datatype};

    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }

    return datatype;
}

MPI_Datatype stat_check_gs_event_datatype() {
    MPI_Datatype datatype;

    int block_lengths[2] = {1, 1};
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    MPI_Get_address(&((struct stat_check_gs *) 0)->gs_rank, &displacements[0]);
    MPI_Get_address(&((struct stat_check_gs *) 0)->distance, &displacements[1]);
    MPI_Get_address(NULL, &base_address);

    // Adjust displacements relative to the base address
    for (int i = 0; i < 2; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[2] = {MPI_INT, MPI_DOUBLE};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }

    return datatype;
}

MPI_Datatype coordinates_datatype() {
    MPI_Datatype coord_type;
    MPI_Type_contiguous(3, MPI_FLOAT, &coord_type);
    return coord_type;
}

MPI_Datatype add_st_metric_event_datatype() {
    MPI_Datatype datatype;

    int block_lengths[3] = {1, 1, DATETIME_MAX_LENGTH};

    // Calculate displacements for each member of the struct
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    MPI_Get_address(&((st_add_metric *) 0)->st_rank, &displacements[0]);
    MPI_Get_address(&((st_add_metric *) 0)->temperature, &displacements[1]);
    MPI_Get_address(&((st_add_metric *) 0)->timestamp, &displacements[2]);
    MPI_Get_address(NULL, &base_address);
    // Adjust displacements relative to the base address
    for (int i = 0; i < 3; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[3] = {MPI_INT, MPI_FLOAT, MPI_CHAR};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(3, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype st_lelect_probe_event_datatype() {
    MPI_Datatype datatype;
    int block_lengths[3] = {1, 1, 1};
    // Calculate displacements for each member of the struct
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    MPI_Get_address(&((st_lelect_probe *) 0)->rank, &displacements[0]);
    MPI_Get_address(&((st_lelect_probe *) 0)->phase, &displacements[1]);
    MPI_Get_address(&((st_lelect_probe *) 0)->hop, &displacements[2]);
    MPI_Get_address(NULL, &base_address);

    // Adjust displacements relative to the base address
    for (int i = 0; i < 3; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(3, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype st_lelect_reply_event_datatype() {
    MPI_Datatype datatype;
    int block_lengths[2] = {1, 1};
    // Calculate displacements for each member of the struct
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    MPI_Get_address(&((st_lelect_reply *) 0)->rank, &displacements[0]);
    MPI_Get_address(&((st_lelect_reply *) 0)->phase, &displacements[1]);
    MPI_Get_address(NULL, &base_address);

    // Adjust displacements relative to the base address
    for (int i = 0; i < 2; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype avg_earth_temp_req_event_datatype() {
    MPI_Datatype datatype;
    int block_lengths[1] = {DATETIME_MAX_LENGTH};
    // Calculate displacements for each member of the struct
    MPI_Aint displacements[1];
    MPI_Aint base_address;
    MPI_Get_address(&((avg_earth_temp_request *) 0)->timestamp, &displacements[0]);
    MPI_Get_address(NULL, &base_address);

    // Define datatypes for each member of the struct
    MPI_Datatype types[1] = {MPI_CHAR};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(1, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype avg_earth_temp_res_event_datatype() {
    MPI_Datatype datatype;
    int block_lengths[4] = {DATETIME_MAX_LENGTH, 1, 1, 1};
    // Calculate displacements for each member of the struct
    MPI_Aint displacements[4];
    MPI_Aint base_address;
    MPI_Get_address(&((avg_earth_temp *) 0)->timestamp, &displacements[0]);
    MPI_Get_address(&((avg_earth_temp *) 0)->avg_temperature, &displacements[1]);
    MPI_Get_address(&((avg_earth_temp *) 0)->st_leader, &displacements[2]);
    MPI_Get_address(&((avg_earth_temp *) 0)->num_of_actual_measurements, &displacements[3]);
    MPI_Get_address(NULL, &base_address);

    // Adjust displacements relative to the base address
    for (int i = 0; i < 4; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[4] = {MPI_CHAR, MPI_DOUBLE, MPI_INT, MPI_INT};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(4, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}

MPI_Datatype sync_event_datatype() {
    MPI_Datatype datatype;
    int block_lengths[2] = {1,1};
    // Calculate displacements for each member of the struct
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    MPI_Get_address(&((sync *) 0)->gs_leader, &displacements[0]);
    MPI_Get_address(&((sync *) 0)->total_checks, &displacements[1]);
    MPI_Get_address(NULL, &base_address);

    // Adjust displacements relative to the base address
    for (int i = 0; i < 2; i++) {
        displacements[i] -= base_address;
    }

    // Define datatypes for each member of the struct
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};

    // Create the MPI datatype for the struct
    const int errcode = MPI_Type_create_struct(2, block_lengths, displacements, types, &datatype);
    if (errcode != MPI_SUCCESS) {
        debug_printf("Failed to create MPI_Datatype for custom event!");
    }
    return datatype;
}
