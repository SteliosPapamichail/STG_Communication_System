//
// Created by fresh on 6-5-24.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include "coordinator.h"

gs_connect parse_connect_event(const char *line, int source_rank) {
    gs_connect data;
    char *str = strtok((char*)line, " "); // Cast line to avoid warnings

    int count = 0;
    int rank;
    while (str != NULL) {
        if (count > 0) {
            if (sscanf(str, "%d", &rank) != 1) {
                printf("");
                MPI_Finalize();
                exit(-1);
            } else {
                if (count == 1) {
                    data.gs_rank = rank;
                } else if (count == 2) {
                    data.neighbor_gs_rank = rank;
                }
            }
        }
        str = strtok(NULL, " ");
        count++;
    }

    data.event_source = source_rank;
    return data;
}