//
// Created by fresh on 6-5-24.
//

#ifndef STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H
#define STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H

#include <mpi/mpi.h>
#include "event_payloads.h"

MPI_Datatype connect_event_datatype(gs_connect event);

#endif //STG_COMMUNICATION_SYSTEM_MPI_DATATYPES_H