//
// Created by fresh on 1-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_SATELLITE_H
#define STG_COMMUNICATION_SYSTEM_SATELLITE_H

float get_st_status();

void set_st_status(float status);

void set_st_coords(const float *coords);

float* get_st_coords();

int get_left_ring_neighbor(int rank, int total_nodes);

int get_right_ring_neighbor(int rank, int total_nodes);

#endif //STG_COMMUNICATION_SYSTEM_SATELLITE_H
