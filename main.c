//
// Created by csd4020 on 27-4-24.
//
#include <stdio.h>
#include <stdlib.h>
#include "common/constants.h"
#include "common/event_payloads.h"
#include "coordinator/coordinator.h"
#include "common/mpi_datatypes.h"
#include "ground_station/ground_station.h"
#include <mpi/mpi.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int rank, size, n, color;
    MPI_Comm comm_world, comm_group;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate and get N from command line argument
    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: mpirun -np <N+1> %s <N+1> <input file>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    n = atoi(argv[1]);
    if (n <= 0 || size != n) {
        if (rank == 0) {
            printf("Invalid arguments. N must be positive and total processes (N+1) must match.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Assign ranks to groups based on their position relative to N
    if (rank == n - 1) {
        color = COORDINATOR_GROUP_ID;
    } else if (rank < n / 2) {
        color = SATELLITE_GROUP_ID; // processes 0 to N/2-1
    } else {
        color = GROUND_STATION_GROUP_ID; // processes N/2 to N-1
    }

    // Create communicator groups
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &comm_group);

    // create custom MPI data types and commit them so they can be used
    MPI_Datatype connect_datatype = connect_event_datatype();
    MPI_Type_commit(&connect_datatype);

    // Identify group information within each group
    int group_rank, group_size;
    MPI_Comm_rank(comm_group, &group_rank);
    MPI_Comm_size(comm_group, &group_size);

    // Print information about each process' group
    printf("Process [rank %d, color %d, group_rank %d, group_size %d]\n",
           rank, color, group_rank, group_size);

    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        MPI_Finalize();
        return 1;
    }

    // Parse input file if we are the coordinator
    if (rank == n - 1) {
        char line[MAX_LINE_LENGTH];
        char event[MAX_LINE_LENGTH];

        //todo move to coordinator
        int connect_ack_count = 0;
        MPI_Status status;

        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) { // read line by line
            // Remove trailing newline character
            line[strcspn(line, "\n")] = '\0';
            // Extract information based on the line type
            sscanf(line, "%s", event); // Read event type

            int is_connect_event = strcmp(event, "CONNECT");

            if (is_connect_event == 0) {
                gs_connect event_data = parse_connect_event(line, rank);
                // send CONNECT to gs_rank
                printf("Coordinator sending connect.\n");
                MPI_Send(&event_data, 1, connect_datatype, event_data.gs_rank, CONNECT, MPI_COMM_WORLD);
            }

            printf("proceeding after connect in coordinator loop. \n");
            if (is_connect_event != 0) { // found different event, so we wait for n/2 ack
                printf("did not find connect event. \n");
                while (connect_ack_count < (n - 1) / 2) {
                    printf("coordinator waiting...\n");
                    // receive ACK events for CONNECT sends
                    MPI_Recv((void *) 0, 0, MPI_INT, MPI_ANY_SOURCE, ACK, MPI_COMM_WORLD, &status);

                    if (status.MPI_TAG == ACK) {
                        printf("coordinator got ACK\n");
                        connect_ack_count++;
                    }
                }
            } else {
                continue;
            }

            // parse other events after CONNECT events have finished
            if (strcmp(event, "ADD_STATUS") == 0) {

            } else if (strcmp(event, "ADD_ST_COORDINATES") == 0) {

            } else if (strcmp(event, "ADD_METRIC") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "ADD_GS_COORDINATES") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "STATUS_CHECK") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "AVG_EARTH_TEMP") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "SYNC") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "PRINT") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "START_LELECT_ST") == 0) {

            } else if (strcmp(event, "START_LELECT_GS") == 0) {

            } else {
                // or could ignore the event and continue
                printf("Invalid event type %s found in file!\n", event);
                MPI_Finalize();
                return 1;
            }
        }

        // if reached, means fgets() reached EOF and got NULL
        if (ferror(file)) {
            printf("Error reading file.\n");
            MPI_Finalize();
            return 1;
        } else {
            //TODO:sp implement TERMINATE messaging
        }
    }

    // if current process is a Satellite
    if (rank >= 0 && rank <= n / 2 - 1) {

    }
    // if it is a Ground Station
    if (rank >= n / 2 && rank <= n - 1) {
        // connect message
        // avg temp message
        // status check message

        // Handle CONNECT events and use the group-ranks for gs-to-gs communication
        gs_connect received_data;
        MPI_Status status;
        // receive connect event from coordinator
        MPI_Recv(&received_data, 1, connect_datatype, n - 1, CONNECT, MPI_COMM_WORLD, &status);

        if (received_data.event_source == n - 1) {
            printf("STG %d (local) got connect from coordinator\n", group_rank);
            // add as parent with group rank, not global rank
            add_parent_gs(received_data.neighbor_gs_rank % group_size);
            received_data.event_source = group_rank;
            // notify parent that we are its neighbor
            MPI_Send(&received_data, 1, connect_datatype, received_data.neighbor_gs_rank, CONNECT, comm_group);
        }

        // receive connect event from other sources in the gs comm group (should be from other ground stations)
        MPI_Recv(&received_data, 1, connect_datatype, MPI_ANY_SOURCE, CONNECT, comm_group, &status);

        if (status.MPI_TAG == CONNECT) {
            if (received_data.event_source == (received_data.gs_rank %
                                               group_size)) { // add as child since the event came from the gs_rank source
                printf("GS neighbor %d (local) got CONNECT from gs_rank %d (local). Sending ACK \n", group_rank, received_data.event_source);
                add_neighbor_gs(received_data.event_source); // add neighbor with group rank
                MPI_Send((void *) 0, 0, MPI_INT, received_data.event_source, ACK, comm_group);
            }
        }

        // receive ack response from neighbor_gs_rank
        MPI_Recv((void *) 0, 0, MPI_INT, MPI_ANY_SOURCE, ACK, comm_group, &status);

        if (status.MPI_TAG == ACK) {
            printf("GS rank %d (local) got ACK from neighbor %d (local). Sending ACK TO COORDINATOR. \n", group_rank, status.MPI_SOURCE % group_size);
            // gs_rank got ACK from neighbor_gs_rank
            // send ACK back to the coordinator
            MPI_Send((void *) 0, 0, MPI_INT, n - 1, ACK, MPI_COMM_WORLD);
        }
    }

    MPI_Type_free(&connect_datatype);
    MPI_Comm_free(&comm_group);

    MPI_Finalize();

    return 0;
}