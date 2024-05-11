//
// Created by fresh on 2-5-24.
//

#include <stdio.h>
#include <malloc.h>
#include "ground_station.h"
#include "../common/event_payloads.h"

int parent_gs;
int *neighbor_gs = NULL;
int num_of_neighbors = 0;
float station_coordinates[3];
int st_leader_rank;

void add_gs_coords(const float *coords) {
    for (int i = 0; i < 3; i++) station_coordinates[i] = coords[i];
}

float *get_gs_coords() {
    return station_coordinates;
}

void add_parent_gs(const int parent_rank) {
    parent_gs = parent_rank;
}

void add_neighbor_gs(const int child_rank) {
    neighbor_gs = realloc(neighbor_gs, (num_of_neighbors + 1) * sizeof(int));
    neighbor_gs[num_of_neighbors++] = child_rank;
}

void set_st_leader(const int leader) {
    st_leader_rank = leader;
}

int get_st_leader() {
    return st_leader_rank;
}
