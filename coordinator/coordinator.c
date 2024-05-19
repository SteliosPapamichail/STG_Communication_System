//
// Created by fresh on 6-5-24.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "coordinator.h"
#include "../common/constants.h"

extern int MAX_LINE_LENGTH;

int st_leader;
int gs_leader;

void set_st_leader_coordinator(const int rank) {
    st_leader = rank;
}

int get_st_leaader_coordinator() {
    return st_leader;
}

void set_gs_leader_coordinator(const int rank) {
    gs_leader = rank;
}

int get_gs_leader_coordinator() {
    return gs_leader;
}

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

gs_add_coords parse_gs_add_coords_event(const char *line) {
    gs_add_coords data;

    const char *token = strtok((char *) line, " ");
    token = strtok(NULL, " "); // skip event

    // could use loops here :)
    data.gs_rank = atoi(token);
    token = strtok(NULL, " "); // skip event
    data.coords[0] = atof(token); // lat
    token = strtok(NULL, " ");
    data.coords[1] = atof(token); // longitude
    token = strtok(NULL, " ");
    data.coords[2] = atof(token); // alt
    return data;
}

status_check parse_status_check(const char *line) {
    status_check data;
    const char *token = strtok((char *) line, " ");
    token = strtok(NULL, " "); // skip event

    // could use loops here :)
    data.st_rank = atoi(token);
    token = strtok(NULL, " "); // skip event
    data.st_coords[0] = atof(token); // lat
    token = strtok(NULL, " ");
    data.st_coords[1] = atof(token); // longitude
    token = strtok(NULL, " ");
    data.st_coords[2] = atof(token); // alt
    return data;
}

st_add_metric parse_st_add_metric_event(const char *line) {
    st_add_metric data;
    // Tokenize the line
    const char *token = strtok((char *) line, " ");

    // Skip the first token/event (ADD_METRIC)
    token = strtok(NULL, " ");
    data.st_rank = atoi(token);
    token = strtok(NULL, " ");
    data.temperature = atof(token);

    // Parse time
    token = strtok(NULL, " ");
    const size_t time_len = strlen(token);
    if (time_len >= DATETIME_MAX_LENGTH) {
        // Handle error: Time string exceeds maximum length
        return data; // Returning empty or default data
    }
    strncpy(data.timestamp, token, time_len);
    data.timestamp[time_len] = '\0'; // Null-terminate the time part

    // Parse date
    token = strtok(NULL, " ");
    const size_t date_len = strlen(token);
    if (date_len + time_len + 1 > DATETIME_MAX_LENGTH) {
        // Handle error: Combined length of time and date exceeds maximum length
        return data; // Returning empty or default data
    }
    // Concatenate space and date to the timestamp buffer
    strcat(data.timestamp, " "); // Add space separator
    strncat(data.timestamp + time_len + 1, token, date_len); // Concatenate date
    data.timestamp[time_len + date_len] = '\0'; // Null-terminate the timestamp
    return data;
}

avg_earth_temp_request parse_avg_earth_temp(char *line) {
    avg_earth_temp_request data;

    // Tokenize the line
    const char *token = strtok((char *) line, " ");

    // Parse time
    token = strtok(NULL, " ");
    const size_t time_len = strlen(token);
    if (time_len >= DATETIME_MAX_LENGTH) {
        // Handle error: Time string exceeds maximum length
        return data; // Returning empty or default data
    }
    strncpy(data.timestamp, token, time_len);
    data.timestamp[time_len] = '\0'; // Null-terminate the time part

    // Parse date
    token = strtok(NULL, " ");
    const size_t date_len = strlen(token);
    if (date_len + time_len + 1 > DATETIME_MAX_LENGTH) {
        // Handle error: Combined length of time and date exceeds maximum length
        return data; // Returning empty or default data
    }
    // Concatenate space and date to the timestamp buffer
    strcat(data.timestamp, " "); // Add space separator
    strncat(data.timestamp + time_len + 1, token, date_len); // Concatenate date
    data.timestamp[time_len + date_len] = '\0'; // Null-terminate the timestamp
    return data;
}