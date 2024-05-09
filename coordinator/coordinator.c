//
// Created by fresh on 6-5-24.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include "coordinator.h"

gs_connect parse_connect_event(const char *line) {
    gs_connect data;
    const char *token = strtok((char *) line, " "); // Cast line to avoid warnings
    token = strtok(NULL, " ");
    data.gs_rank = atoi(token);
    token = strtok(NULL, " ");
    data.neighbor_gs_rank = atoi(token);
    return data;
}

st_add_status parse_add_status_event(const char *line) {
    st_add_status data;

    // Tokenize the line
    const char *token = strtok((char *) line, " ");

    token = strtok(NULL, " ");
    // Parse the first token (integer)
    data.st_rank = atoi(token);

    // Parse the second token (float)
    token = strtok(NULL, " ");
    data.status = atof(token);

    return data;
}

st_add_coords parse_st_add_coords_event(const char *line) {
    st_add_coords data;

    const char *token = strtok((char *) line, " ");
    token = strtok(NULL, " "); // skip event

    // could use loops here :)
    data.st_rank = atoi(token);
    token = strtok(NULL, " "); // skip event
    data.coords[0] = atof(token); // lat
    token = strtok(NULL, " ");
    data.coords[1] = atof(token); // longitude
    token = strtok(NULL, " ");
    data.coords[2] = atof(token); // alt
    return data;
}
