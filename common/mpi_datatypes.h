//
// Created by fresh on 6-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H
#define STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H

#include <mpi/mpi.h>
#include "event_payloads.h"

MPI_Datatype connect_event_datatype();

MPI_Datatype add_status_event_datatype();

MPI_Datatype add_st_coords_event_datatype(MPI_Datatype);

MPI_Datatype add_gs_coords_event_datatype(MPI_Datatype);

MPI_Datatype coordinates_datatype();

MPI_Datatype add_st_metric_event_datatype();

MPI_Datatype st_lelect_probe_event_datatype();

MPI_Datatype st_lelect_reply_event_datatype();

MPI_Datatype status_check_event_datatype(MPI_Datatype);

MPI_Datatype stat_check_gs_event_datatype();

MPI_Datatype avg_earth_temp_req_event_datatype();

MPI_Datatype avg_earth_temp_res_event_datatype();

#endif //STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H
