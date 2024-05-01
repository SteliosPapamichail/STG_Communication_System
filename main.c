//
// Created by csd4020 on 27-4-24.
//
#include <stdio.h>
#include <stdlib.h>
#include "common/constants.h"
#include <mpi/mpi.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int rank, size, n, color;
    MPI_Comm comm_world, comm_group;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate and get N from command line argument
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: mpirun -np <N+1> %s <N>\n", argv[0]);
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

    // Identify group information within each group
    int group_rank, group_size;
    MPI_Comm_rank(comm_group, &group_rank);
    MPI_Comm_size(comm_group, &group_size);

    // Print information about each process' group
    printf("Process [rank %d, color %d, group_rank %d, group_size %d]\n",
           rank, color, group_rank, group_size);

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        MPI_Finalize();
        return 1;
    }

    // Parse input file if we are the coordinator
    if (rank == n) {
        char line[MAX_LINE_LENGTH];
        char event[MAX_LINE_LENGTH];

        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) { // read line by line
            // Remove trailing newline character
            line[strcspn(line, "\n")] = '\0';

            // Extract information based on the line type
            sscanf(line, "%s", event); // Read event type
            if (strcmp(event, "CONNECT") == 0) {
                // Extract data for CONNECT line (modify based on your needs)
                sscanf(line, "%s %d %d", line_info.type, &line_info.data1, &line_info.data2);
            } else if (strcmp(event, "ADD_STATUS") == 0) {
                // Extract data for ADD_STATUS line (modify based on your needs)
                sscanf(line, "%s %d %lf", line_info.type, &line_info.data1, &line_info.data2);
            } else if (strcmp(event, "ADD_ST_COORDINATES") == 0) {
                // Extract data for ADD_ST_COORDINATES line (modify based on your needs)
                sscanf(line, "%s %d %lf %lf %d", line_info.type, &line_info.data1,
                       &line_info.data2, &line_info.data3, &line_info.data2);
            } else if (strcmp(event, "ADD_METRIC") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "ADD_STATUS") == 0) {
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
            } else {
                // or could ignore the event and continue
                printf("Invalid event type %s found in file!\n", event);
                MPI_Finalize();
                return 1;
            }
        }

        // check if we reached the EOF so we can terminate
        if (line == NULL) {
            if (ferror(file)) {
                printf("Error reading file.\n");
                MPI_Finalize();
                return 1;
            } else {
                //TODO:sp implement TERMINATE messaging
            }
        }
    }

    MPI_Comm_free(&comm_group);
    MPI_Finalize();

    return 0;
}