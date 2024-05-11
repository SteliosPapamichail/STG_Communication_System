//
// Created by csd4020 on 27-4-24.
//
#include <stdio.h>
#include <stdlib.h>
#include "common/constants.h"
#include "common/event_payloads.h"
#include "coordinator/coordinator.h"
#include "common/mpi_datatypes.h"
#include "ground_station/ground_station.h"
#include <mpi/mpi.h>
#include <string.h>

#include "common/temp_metrics.h"
#include "satellite/satellite.h"

#define CONNECT_ACK_TOTAL(x) (((x - 1)/2) - 1)
const int MAX_LINE_LENGTH = 256;

int main(int argc, char *argv[]) {
    int rank, size, color;
    MPI_Comm comm_group;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate and get N from command line argument
    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: mpirun -np <N+1> %s <N> <input file>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    const int n = atoi(argv[1]) + 1;
    if (n <= 0 || size != n) {
        if (rank == 0) {
            printf("Invalid arguments. N must be positive and total processes (N+1) must match.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Assign ranks to groups based on their position relative to N
    if (rank == n - 1) {
        color = COORDINATOR_GROUP_ID;
    } else if (rank < (n - 1) / 2) {
        color = SATELLITE_GROUP_ID; // processes 0 to N/2-1
    } else {
        color = GROUND_STATION_GROUP_ID; // processes N/2 to N-1
    }

    // Create communicator groups
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &comm_group);

    // create custom MPI data types and commit them so they can be used
    MPI_Datatype connect_datatype = connect_event_datatype();
    MPI_Datatype add_status_datatype = add_status_event_datatype();
    MPI_Datatype coords_datatype = coordinates_datatype();
    MPI_Datatype st_add_coords_datatype = add_st_coords_event_datatype(coords_datatype);
    MPI_Datatype gs_add_coords_datatype = add_gs_coords_event_datatype(coords_datatype);
    MPI_Datatype st_add_metric_datatype = add_st_metric_event_datatype();
    MPI_Datatype st_lelect_probe_datatype = st_lelect_probe_event_datatype();
    MPI_Datatype st_lelect_reply_datatype = st_lelect_reply_event_datatype();
    MPI_Type_commit(&connect_datatype);
    MPI_Type_commit(&add_status_datatype);
    MPI_Type_commit(&coords_datatype);
    MPI_Type_commit(&st_add_coords_datatype);
    MPI_Type_commit(&gs_add_coords_datatype);
    MPI_Type_commit(&st_add_metric_datatype);
    MPI_Type_commit(&st_lelect_probe_datatype);
    MPI_Type_commit(&st_lelect_reply_datatype);

    // Identify group information within each group
    int group_rank, group_size;
    MPI_Comm_rank(comm_group, &group_rank);
    MPI_Comm_size(comm_group, &group_size);

    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        MPI_Finalize();
        return 1;
    }

    // Parse input file if we are the coordinator
    if (rank == n - 1) {
        char line[MAX_LINE_LENGTH];

        //todo move to coordinator
        int connect_ack_count = 0;
        MPI_Status status;

        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
            char event[MAX_LINE_LENGTH];
            // read line by line
            // Remove trailing newline character
            line[strcspn(line, "\n")] = '\0';
            // Extract information based on the line type
            sscanf(line, "%s", event); // Read event type

            const int is_connect_event = strcmp(event, "CONNECT");

            if (is_connect_event == 0) {
                gs_connect event_data = parse_connect_event(line);
                // send CONNECT to gs_rank
                MPI_Send(&event_data, 1, connect_datatype, event_data.gs_rank, CONNECT, MPI_COMM_WORLD);
            }

            if (is_connect_event != 0) {
                // found different event, so we wait for n/2 ack if needed
                while (connect_ack_count < CONNECT_ACK_TOTAL(n)) {
                    // receive ACK events for CONNECT sends
                    MPI_Recv((void *) 0, 0, MPI_INT, MPI_ANY_SOURCE, ACK, MPI_COMM_WORLD, &status);
                    connect_ack_count++;
                }
            } else {
                continue;
            }

            printf("parsing %s\n", event);

            // parse other events after CONNECT events have finished
            if (strcmp(event, "ADD_STATUS") == 0) {
                st_add_status event_data = parse_add_status_event(line);
                MPI_Send(&event_data, 1, add_status_datatype, event_data.st_rank, ADD_STATUS, MPI_COMM_WORLD);
            } else if (strcmp(event, "ADD_ST_COORDINATES") == 0) {
                st_add_coords event_data = parse_st_add_coords_event(line);
                MPI_Send(&event_data, 1, st_add_coords_datatype, event_data.st_rank, ADD_ST_COORDINATES,
                         MPI_COMM_WORLD);
            } else if (strcmp(event, "ADD_METRIC") == 0) {
                st_add_metric event_data = parse_st_add_metric_event(line);
                MPI_Send(&event_data, 1, st_add_metric_datatype, event_data.st_rank, ADD_METRIC, MPI_COMM_WORLD);
            } else if (strcmp(event, "ADD_GS_COORDINATES") == 0) {
                gs_add_coords event_data = parse_gs_add_coords_event(line);
                MPI_Send(&event_data, 1, gs_add_coords_datatype, event_data.gs_rank, ADD_GS_COORDINATES,
                         MPI_COMM_WORLD);
            } else if (strcmp(event, "STATUS_CHECK") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "AVG_EARTH_TEMP") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "SYNC") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "PRINT") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "START_LELECT_ST") == 0) {
                //todo: replace with mpi_scatter probably to improve network utilization
                printf("-=-=-=-=-=-=- STARTING ELECTION PROCESS -=-=-=-=-=-=-=-\n");
                for (int i = 0; i < (n - 1) / 2; i++) {
                    printf("sending start elect to st %d\n", i);
                    MPI_Send(NULL, 0, MPI_INT, i, START_LELECT_ST, MPI_COMM_WORLD);
                }

                int st_leader_rank;
                // wait for leader id
                MPI_Recv(&st_leader_rank, 1, MPI_INT, MPI_ANY_SOURCE, LELECT_ST_DONE, MPI_COMM_WORLD, &status);
                // forward to all GS processes with broadcast ig
                for (int i = (n - 1) / 2; i < n - 1; i++) {
                    printf("sending st leader rank to GS %d\n", i);
                    MPI_Send(&st_leader_rank, 1, MPI_INT, i, ST_LEADER, MPI_COMM_WORLD);
                }
            } else if (strcmp(event, "START_LELECT_GS") == 0) {
            } else if (strcmp(event, "TERMINATE") == 0) {
            } else {
                // or could ignore the event and continue
                printf("Invalid event type %s found in file!\n", event);
                MPI_Finalize();
                return 1;
            }
        }

        // if reached, means fgets() reached EOF and got NULL
        if (ferror(file)) {
            printf("Error reading file.\n");
            MPI_Finalize();
            return 1;
        } else {
            //TODO:sp implement TERMINATE messaging
        }
    }

    // if current process is a Satellite
    if (rank >= 0 && rank <= (n - 1) / 2 - 1) {
        MPI_Status status_world;
        int probe_world_flag;
        int leader_election_done = 0;
        metrics_list *metrics = create_metrics_list();

        do {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe_world_flag, &status_world);

            if (probe_world_flag) {
                if (status_world.MPI_TAG == ADD_STATUS && status_world.MPI_SOURCE == n - 1) {
                    st_add_status received_data;

                    MPI_Recv(&received_data, 1, add_status_datatype, n - 1, ADD_STATUS, MPI_COMM_WORLD, &status_world);

                    set_st_status(received_data.status);
                    //printf("SATELLITE %d got status %.1f\n", rank, get_st_status());
                } else if (status_world.MPI_TAG == ADD_ST_COORDINATES && status_world.MPI_SOURCE == n - 1) {
                    st_add_coords received_data;
                    MPI_Recv(&received_data, 1, st_add_coords_datatype, n - 1, ADD_ST_COORDINATES, MPI_COMM_WORLD,
                             &status_world);
                    set_st_coords(received_data.coords);
                    //float *temp = get_st_coords();
                    //printf("SATELLITE %d got COORDINATES %.4f , %.4f , %.4f\n", rank, temp[0], temp[1], temp[2]);
                } else if (status_world.MPI_TAG == ADD_METRIC && status_world.MPI_SOURCE == n - 1) {
                    st_add_metric received_data;
                    MPI_Recv(&received_data, 1, st_add_metric_datatype, n - 1, ADD_METRIC, MPI_COMM_WORLD,
                             &status_world);
                    add_metric(metrics, received_data);
                } else if (status_world.MPI_TAG == START_LELECT_ST && status_world.MPI_SOURCE == n - 1 && !
                           leader_election_done) {
                    // start election process
                    printf("ST %d got start elect with tag %d and flag %d\n", rank, status_world.MPI_TAG,
                           probe_world_flag);
                    MPI_Barrier(comm_group);
                    perform_st_leader_election(n - 1, group_rank, group_size, comm_group, st_lelect_probe_datatype,
                                               st_lelect_reply_datatype);
                    leader_election_done = 1;
                }
            }
        } while (status_world.MPI_TAG != TERMINATE);

        // received TERMINATE
        destroy_metrics_list(metrics);
    }
    // if it is a Ground Station
    if (rank >= (n - 1) / 2 && rank <= n - 1) {
        // connect message
        // avg temp message
        // status check message
        MPI_Status status_world, status_gs;
        int probe_world_flag, probe_gs_flag;

        do {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe_world_flag, &status_world);

            if (probe_world_flag) {
                if (status_world.MPI_TAG == CONNECT && status_world.MPI_SOURCE == n - 1) {
                    // Handle CONNECT from coordinator
                    gs_connect received_data;

                    // receive connect event from coordinator
                    MPI_Recv(&received_data, 1, connect_datatype, n - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status_world);

                    printf("GS %d got connect from coordinator\n", rank);
                    // add as parent with group rank, not global rank
                    add_parent_gs(received_data.neighbor_gs_rank % group_size);
                    // notify parent that we are its neighbor
                    MPI_Send(&received_data, 1, connect_datatype, received_data.neighbor_gs_rank % group_size, CONNECT,
                             comm_group);

                    // receive ack response from neighbor_gs_rank
                    printf("%d waiting to receive ACK from %d\n", rank, received_data.neighbor_gs_rank);
                    MPI_Recv((void *) 0, 0, MPI_INT, received_data.neighbor_gs_rank % group_size, ACK, comm_group,
                             &status_world);

                    if (status_world.MPI_TAG == ACK) {
                        printf("GS rank %d (local) got ACK from neighbor %d (local). Sending ACK TO COORDINATOR. \n",
                               group_rank,
                               status_world.MPI_SOURCE % group_size);
                        // gs_rank got ACK from neighbor_gs_rank
                        // send ACK back to the coordinator
                        MPI_Send((void *) 0, 0, MPI_INT, n - 1, ACK, MPI_COMM_WORLD);
                    }
                } else if (status_world.MPI_TAG == ADD_GS_COORDINATES && status_world.MPI_SOURCE == n - 1) {
                    gs_add_coords received_data;
                    MPI_Recv(&received_data, 1, gs_add_coords_datatype, n - 1, ADD_GS_COORDINATES, MPI_COMM_WORLD,
                             &status_world);

                    add_gs_coords(received_data.coords);
                    //float *temp = get_gs_coords();
                    //printf("STATION %d got COORDINATES %.6f , %.6f , %.6f\n", rank, temp[0], temp[1], temp[2]);
                } else if (status_world.MPI_TAG == ST_LEADER && status_world.MPI_SOURCE == n - 1) {
                    int st_leader_rank;
                    MPI_Recv(&st_leader_rank, 1, MPI_INT, n - 1, ST_LEADER, MPI_COMM_WORLD, &status_world);
                    printf("GS %d storing leader %d\n", rank, st_leader_rank);
                    set_st_leader(st_leader_rank);
                }
            }

            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm_group, &probe_gs_flag, &status_gs);

            if (probe_gs_flag) {
                if (status_gs.MPI_TAG == CONNECT) {
                    // got CONNECT from another GS
                    gs_connect received_data;

                    MPI_Recv(&received_data, 1, connect_datatype, MPI_ANY_SOURCE, CONNECT, comm_group, &status_gs);

                    if (status_gs.MPI_SOURCE == received_data.gs_rank % group_size) {
                        // add as child since the event came from the gs_rank source
                        printf("GS neighbor %d got CONNECT from gs_rank %d . Sending ACK to parent %d.\n",
                               rank,
                               received_data.gs_rank, status_gs.MPI_SOURCE + group_size);
                        add_neighbor_gs(status_gs.MPI_SOURCE); // add neighbor with group rank
                        MPI_Send((void *) 0, 0, MPI_INT, status_gs.MPI_SOURCE, ACK, comm_group);
                    }
                }
            }
        } while (status_world.MPI_TAG != TERMINATE);
    }

    MPI_Type_free(&connect_datatype);
    MPI_Type_free(&add_status_datatype);
    MPI_Type_free(&coords_datatype);
    MPI_Type_free(&st_add_coords_datatype);
    MPI_Type_free(&gs_add_coords_datatype);
    MPI_Type_free(&st_add_metric_datatype);
    MPI_Type_free(&st_lelect_probe_datatype);
    MPI_Type_free(&st_lelect_reply_datatype);
    MPI_Comm_free(&comm_group);

    MPI_Finalize();

    return 0;
}
