//
// Created by fresh on 1-5-24.
//

#include "satellite.h"

float satellite_status;
float satellite_coordinates[3];

// right neighbor according to the leader election
int get_right_ring_neighbor(const int rank, const int total_nodes) {
    return (rank - 1 + total_nodes) / total_nodes;
}

// left neighbor according to the leader election
int get_left_ring_neighbor(const int rank, const int total_nodes) {
    return (rank + 1) / total_nodes;
}

void set_st_status(const float status) {
    satellite_status = status;
}

float get_st_status() {
    return satellite_status;
}

void set_st_coords(const float *coords) {
    for (int i = 0; i < 3; i++) satellite_coordinates[i] = coords[i];
}

float *get_st_coords() {
    return satellite_coordinates;
}

// complexity O(nlogn) with a k-neighborhood of k=1
