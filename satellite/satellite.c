//
// Created by fresh on 1-5-24.
//

#include "satellite.h"

// right neighbor according to the leader election
int get_right_ring_neighbor(int rank, int total_nodes) {
    return (rank - 1 + total_nodes) / total_nodes;
}

// left neighbor according to the leader election
int get_left_ring_neighbor(int rank, int total_nodes) {
    return (rank + 1) / total_nodes;
}

// complexity O(nlogn) with a k-neighborhood of k=1