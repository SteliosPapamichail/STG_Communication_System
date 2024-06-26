//
// Created by fresh on 1-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_EVENT_PAYLOADS_H
#define STG_COMMUNICATION_SYSTEM_EVENT_PAYLOADS_H

#define _XOPEN_SOURCE
#include <time.h>

#include "constants.h"

typedef struct e_gs_connection_info {
    int gs_rank;
    int neighbor_gs_rank;
} gs_connect;

/**
 * Represents the payload for the ADD_METRIC event
 */
typedef struct e_add_metric_info {
    int st_rank;
    float temperature;
    char timestamp[DATETIME_MAX_LENGTH];
} st_add_metric;

/**
 * Represents the payload for the ADD_STATUS event
 */
typedef struct e_add_status_info {
    int st_rank;
    float status;
} st_add_status;

/**
 * Represents the payload for the ADD_ST_COORDINATES event
 */
typedef struct e_add_st_coordinates_info {
    int st_rank;
    float coords[3];
} st_add_coords;

/**
 * Represents the payload for the ADD_GS_COORDINATES event
 */
typedef struct e_add_gs_coordinates_info {
    int gs_rank;
    float coords[3];
} gs_add_coords;

/**
 * Represents the payload for the STATUS_CHECK event
 */
typedef struct e_status_check {
    int st_rank;
    float st_coords[3];
} status_check;

/**
 * Represents the payload for the AVG_EARTH_TEMP event
 * request. This payload is sent to initiate the calculation
 * and in turn should lead to an e_avg_earth_temp_response
 * event being emitted back, containing the result.
 */
typedef struct e_avg_earth_temp_request {
    char timestamp[DATETIME_MAX_LENGTH];
} avg_earth_temp_request;

/**
 * Represents the payload for the AVG_EARTH_TEMP_DONE event
 * and contains the requested average temperature calculation
 * for the timestamp initially sent with the e_avg_earth_temp_request
 * event.
 */
typedef struct e_avg_earth_temp_response {
    char timestamp[DATETIME_MAX_LENGTH];
    double avg_temperature;
    int st_leader;
    int num_of_actual_measurements;
} avg_earth_temp;

typedef struct e_sync {
    int gs_leader;
    int total_checks;
} sync;

typedef struct e_st_lelect_probe {
    int rank;
    int phase;
    int hop;
} st_lelect_probe;

typedef struct e_st_lelect_reply {
    int rank;
    int phase;
} st_lelect_reply;

struct stat_check_gs {
    int gs_rank;
    double distance;
};
#endif //STG_COMMUNICATION_SYSTEM_EVENT_PAYLOADS_H
