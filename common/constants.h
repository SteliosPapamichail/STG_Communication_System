//
// Created by fresh on 1-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_CONSTANTS_H
#define STG_COMMUNICATION_SYSTEM_CONSTANTS_H

const int MAX_LINE_LENGTH = 256;

// group ids
enum process_group_id {
    COORDINATOR_GROUP_ID = 0,
    SATELLITE_GROUP_ID = 1,
    GROUND_STATION_GROUP_ID = 2
};

// event/msg/envelope ids
enum event_t {
    ADD_METRIC = 0,
    ADD_STATUS = 1,
    ADD_ST_COORDINATES = 2,
    ADD_GS_COORDINATES = 3,
    STATUS_CHECK = 4,
    AVG_EARTH_TEMP = 5,
    SYNC = 6,
    PRINT = 7,
    TERMINATE = 8,
    FIND_MIN_DIST = 9,
    AVG_EARTH_TEMP_DONE = 10,
    CONNECT = 11,
    ACK = 12
};
#endif //STG_COMMUNICATION_SYSTEM_CONSTANTS_H
