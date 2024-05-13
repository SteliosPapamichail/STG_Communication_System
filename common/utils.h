//
// Created by parallels on 5/12/24.
//

#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdlib.h>

inline double calc_distance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
    double dlat, dlon, a, c, distance;

    // Convert latitudes and longitudes to radians
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;

    // Calculate the difference in latitude and longitude
    dlat = lat2 - lat1;
    dlon = lon2 - lon1;

    // Haversine formula steps
    a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
    c = 2 * atan2(sqrt(a), sqrt(1 - a));

    // Earth radius (replace 6371.0 with your desired radius if applicable)
    const double earth_radius = 6371.0;

    // Calculate the distance in kilometers
    distance = earth_radius * c;

    // we assume altitude of 0 so no difference there
    distance += abs(alt2 - alt1);

    return distance;
}

#endif //UTILS_H
