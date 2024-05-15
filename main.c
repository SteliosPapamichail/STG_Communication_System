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
    MPI_Datatype status_check_datatype = status_check_event_datatype(coords_datatype);
    MPI_Datatype stat_check_gs_datatype = stat_check_gs_event_datatype();
    MPI_Datatype avg_earth_temp_req_datatype = avg_earth_temp_req_event_datatype();
    MPI_Datatype avg_earth_temp_res_datatype = avg_earth_temp_res_event_datatype();
    MPI_Datatype sync_datatype = sync_event_datatype();
    MPI_Type_commit(&connect_datatype);
    MPI_Type_commit(&add_status_datatype);
    MPI_Type_commit(&coords_datatype);
    MPI_Type_commit(&st_add_coords_datatype);
    MPI_Type_commit(&gs_add_coords_datatype);
    MPI_Type_commit(&st_add_metric_datatype);
    MPI_Type_commit(&st_lelect_probe_datatype);
    MPI_Type_commit(&st_lelect_reply_datatype);
    MPI_Type_commit(&status_check_datatype);
    MPI_Type_commit(&stat_check_gs_datatype);
    MPI_Type_commit(&avg_earth_temp_req_datatype);
    MPI_Type_commit(&avg_earth_temp_res_datatype);
    MPI_Type_commit(&sync_datatype);

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

            printf("parsing %s event\n", event);

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
                status_check event_data = parse_status_check(line);
                MPI_Send(&event_data, 1, status_check_datatype, get_gs_leader_coordinator(), FIND_MIN_DIST,
                         MPI_COMM_WORLD);

                //todo:sp add receive so that we wait for completion etc.
            } else if (strcmp(event, "AVG_EARTH_TEMP") == 0) {
                avg_earth_temp_request event_data = parse_avg_earth_temp(line);
                printf("timestamp %s\n", event_data.timestamp);
                MPI_Send(&event_data, 1, avg_earth_temp_req_datatype, get_gs_leader_coordinator(), AVG_EARTH_TEMP,
                         MPI_COMM_WORLD);

                printf("coordinator waiting avg earth temp done\n");
                MPI_Recv(NULL, 0, MPI_INT, get_gs_leader_coordinator(), AVG_EARTH_TEMP_DONE, MPI_COMM_WORLD, &status);
                printf("got avg temp done!\n");
            } else if (strcmp(event, "SYNC") == 0) {
                int gs_leader = get_gs_leader_coordinator();
                printf("------ GS LEADER = %d\n", gs_leader);
                // send SYNC to all GS along with the leader to ease propagation throughout the tree
                for (int i = (n - 1) / 2; i < n - 1; i++) {
                    MPI_Send(&gs_leader, 1, MPI_INT, i, SYNC, MPI_COMM_WORLD);
                }
                int total_status_checks;
                MPI_Recv(&total_status_checks, 1, MPI_INT, gs_leader, SYNC_DONE, MPI_COMM_WORLD, &status);
                printf("[Coordinator (process %d) - SYNC] Total status checks performed: %d\n", rank,
                       total_status_checks);
            } else if (strcmp(event, "PRINT") == 0) {
                // Handle other line types as needed
            } else if (strcmp(event, "START_LELECT_ST") == 0) {
                //todo: replace with mpi_scatter probably to improve network utilization
                for (int i = 0; i < (n - 1) / 2; i++) {
                    MPI_Send((void *) 0, 0, MPI_INT, i, START_LELECT_ST, MPI_COMM_WORLD);
                }

                int st_leader_rank;
                // wait for leader id
                MPI_Recv(&st_leader_rank, 1, MPI_INT, MPI_ANY_SOURCE, LELECT_ST_DONE, MPI_COMM_WORLD, &status);
                set_st_leader_coordinator(st_leader_rank);
                // forward to all GS processes with broadcast ig
                for (int i = (n - 1) / 2; i < n - 1; i++) {
                    //printf("sending st leader rank to GS %d\n", i);
                    MPI_Send(&st_leader_rank, 1, MPI_INT, i, ST_LEADER, MPI_COMM_WORLD);
                }
            } else if (strcmp(event, "START_LELECT_GS") == 0) {
                for (int i = (n - 1) / 2; i < n - 1; i++) {
                    MPI_Send((void *) 0, 0, MPI_INT, i, START_LELECT_GS, MPI_COMM_WORLD);
                }

                int gs_leader_rank;
                // wait for leader id
                MPI_Recv(&gs_leader_rank, 1, MPI_INT, MPI_ANY_SOURCE, LELECT_GS_DONE, MPI_COMM_WORLD, &status);
                // account for GS communicator size
                gs_leader_rank += (n - 1) / 2;
                set_gs_leader_coordinator(gs_leader_rank);
                printf("==== COORDINATOR GOT GS LEADER %d\n", gs_leader_rank);
                // forward gs rank to all ST processes
                for (int i = 0; i < (n - 1) / 2; i++) {
                    MPI_Send(&gs_leader_rank, 1, MPI_INT, i, GS_LEADER, MPI_COMM_WORLD);
                }
            } else if (strcmp(event, "TERMINATE") == 0) {
                for (int i = 0; i < n - 1; i++) {
                    MPI_Send((void *) 0, 0, MPI_INT, i, TERMINATE, MPI_COMM_WORLD);
                }
            }
        }

        if (ferror(file)) {
            printf("Error reading file.\n");
            MPI_Finalize();
            return -1;
        }
    } else if (rank >= 0 && rank < (n - 1) / 2) {
        // if current process is a Satellite
        MPI_Status status_world, status_group;
        int probe_world_flag, probe_group_flag;
        int leader_election_done = 0;
        metrics_list *metrics = create_metrics_list();

        do {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe_world_flag, &status_world);

            if (probe_world_flag) {
                if (status_world.MPI_TAG == ADD_STATUS && status_world.MPI_SOURCE == n - 1) {
                    st_add_status received_data;

                    MPI_Recv(&received_data, 1, add_status_datatype, n - 1, ADD_STATUS, MPI_COMM_WORLD, &status_world);

                    set_st_status(received_data.status);
                } else if (status_world.MPI_TAG == ADD_ST_COORDINATES && status_world.MPI_SOURCE == n - 1) {
                    st_add_coords received_data;
                    MPI_Recv(&received_data, 1, st_add_coords_datatype, n - 1, ADD_ST_COORDINATES, MPI_COMM_WORLD,
                             &status_world);
                    set_st_coords(received_data.coords);
                } else if (status_world.MPI_TAG == ADD_METRIC && status_world.MPI_SOURCE == n - 1) {
                    st_add_metric received_data;
                    MPI_Recv(&received_data, 1, st_add_metric_datatype, n - 1, ADD_METRIC, MPI_COMM_WORLD,
                             &status_world);
                    add_metric(metrics, received_data);
                } else if (status_world.MPI_TAG == START_LELECT_ST && status_world.MPI_SOURCE == n - 1 && !
                           leader_election_done) {
                    // start election process
                    MPI_Recv(NULL, 0, MPI_INT, n - 1, START_LELECT_ST, MPI_COMM_WORLD, &status_world);
                    MPI_Barrier(comm_group);
                    perform_st_leader_election(n - 1, group_rank, group_size, comm_group, st_lelect_probe_datatype,
                                               st_lelect_reply_datatype);
                    leader_election_done = 1;
                } else if (status_world.MPI_TAG == GS_LEADER && status_world.MPI_SOURCE == n - 1) {
                    int gs_leader_rank;
                    MPI_Recv(&gs_leader_rank, 1, MPI_INT, n - 1, GS_LEADER, MPI_COMM_WORLD, &status_world);
                    set_gs_leader_st(gs_leader_rank);
                    MPI_Barrier(comm_group);
                } else if (status_world.MPI_TAG == TERMINATE && status_world.MPI_SOURCE == n - 1) {
                    MPI_Recv(NULL, 0, MPI_INT, n - 1, TERMINATE, MPI_COMM_WORLD, &status_world);
                    break;
                } else if (status_world.MPI_TAG == AVG_EARTH_TEMP) {
                    avg_earth_temp_request req;
                    printf("st leader waiting to receive avg earth start\n");
                    MPI_Recv(&req, 1, avg_earth_temp_req_datatype, status_world.MPI_SOURCE, AVG_EARTH_TEMP,
                             MPI_COMM_WORLD,
                             &status_world);
                    printf("ST leader %d got request for avg temp\n", rank);

                    avg_earth_temp result;
                    float temp = get_temp_for_timestamp(metrics, req.timestamp);
                    strncpy(result.timestamp, req.timestamp, DATETIME_MAX_LENGTH);
                    result.avg_temperature = temp;
                    result.st_leader = group_rank;
                    result.num_of_actual_measurements = 0;
                    if (temp != -1.0f) {
                        // measurement exists
                        result.num_of_actual_measurements = 1;
                    }
                    MPI_Send(&result, 1, avg_earth_temp_res_datatype, get_left_ring_neighbor(group_rank, group_size),
                             AVG_EARTH_TEMP,
                             comm_group);
                    printf("ST leader sent temp to left neighbor %d\n", get_left_ring_neighbor(group_rank, group_size));
                }
            }

            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm_group, &probe_group_flag, &status_group);

            if (probe_group_flag) {
                if (status_group.MPI_TAG == AVG_EARTH_TEMP) {
                    avg_earth_temp data;
                    printf("ST %d waiting to receive avg earth temp request from ring\n", group_rank);
                    MPI_Recv(&data, 1, avg_earth_temp_res_datatype, status_group.MPI_SOURCE, AVG_EARTH_TEMP, comm_group,
                             &status_group);
                    printf("st %d got request from ring\n", group_rank);

                    if (rank == data.st_leader) {
                        // came full circle/ring, notify gs leader
                        data.avg_temperature = data.avg_temperature / data.num_of_actual_measurements;
                        printf("leader got avg temp %f \n", data.avg_temperature);
                        MPI_Send(&data, 1, avg_earth_temp_res_datatype, get_gs_leader_st(), AVG_EARTH_TEMP_DONE,
                                 MPI_COMM_WORLD);
                    } else {
                        // propagate message
                        float temperature = get_temp_for_timestamp(metrics, data.timestamp);
                        data.avg_temperature += temperature;
                        if (temperature != -1.0f) {
                            // measurement exists
                            data.num_of_actual_measurements += 1;
                        }
                        printf("non-leader incrementing temp\n");
                        MPI_Send(&data, 1, avg_earth_temp_res_datatype, get_left_ring_neighbor(group_rank, group_size),
                                 AVG_EARTH_TEMP,
                                 comm_group);
                    }
                }
            }
        } while (1);

        // received TERMINATE
        destroy_metrics_list(metrics);
    } else if (rank >= (n - 1) / 2 && rank <= n - 1) {
        // if it is a Ground Station
        MPI_Status status_world, status_gs;
        int probe_world_flag, probe_gs_flag;
        int leader_election_done = 0;
        int total_checks = 0;
        int num_of_sync_replies = 0;
        metrics_list *avg_metrics = create_metrics_list();

        do {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe_world_flag, &status_world);

            if (probe_world_flag) {
                if (status_world.MPI_TAG == CONNECT && status_world.MPI_SOURCE == n - 1) {
                    // Handle CONNECT from coordinator
                    gs_connect received_data;
                    // receive connect event from coordinator
                    MPI_Recv(&received_data, 1, connect_datatype, n - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status_world);
                    // add as parent with group rank, not global rank
                    add_parent_gs(received_data.neighbor_gs_rank % group_size);
                    // add as neighbor as well
                    add_neighbor_gs(received_data.neighbor_gs_rank % group_size);
                    // notify parent that we are its neighbor
                    MPI_Send(&received_data, 1, connect_datatype, received_data.neighbor_gs_rank % group_size, CONNECT,
                             comm_group);
                } else if (status_world.MPI_TAG == ADD_GS_COORDINATES && status_world.MPI_SOURCE == n - 1) {
                    gs_add_coords received_data;
                    MPI_Recv(&received_data, 1, gs_add_coords_datatype, n - 1, ADD_GS_COORDINATES, MPI_COMM_WORLD,
                             &status_world);
                    add_gs_coords(received_data.coords);
                } else if (status_world.MPI_TAG == ST_LEADER && status_world.MPI_SOURCE == n - 1) {
                    int st_leader_rank;
                    MPI_Recv(&st_leader_rank, 1, MPI_INT, n - 1, ST_LEADER, MPI_COMM_WORLD, &status_world);
                    set_st_leader_gs(st_leader_rank);
                    MPI_Barrier(comm_group);
                } else if (!leader_election_done && status_world.MPI_TAG == START_LELECT_GS && status_world.MPI_SOURCE
                           == n - 1) {
                    MPI_Recv(NULL, 0, MPI_INT, n - 1, START_LELECT_GS, MPI_COMM_WORLD, &status_world);
                    MPI_Barrier(comm_group);
                    perform_gs_leader_election(n - 1, group_rank, comm_group);
                    leader_election_done = 1;
                    MPI_Barrier(comm_group);
                } else if (status_world.MPI_TAG == TERMINATE && status_world.MPI_SOURCE == n - 1) {
                    MPI_Recv(NULL, 0, MPI_INT, n - 1, TERMINATE, MPI_COMM_WORLD, &status_world);
                    break;
                } else if (status_world.MPI_TAG == FIND_MIN_DIST && status_world.MPI_SOURCE == n - 1) {
                    status_check data;
                    MPI_Recv(&data, 1, status_check_datatype, n - 1, FIND_MIN_DIST, MPI_COMM_WORLD, &status_world);
                    printf("(IMPLEMENT FEATURE) Leader GS %d got FIND_MIND_DIST request.\n", group_rank);
                    //todo:sp get_min_dist_gs(group_rank, data, comm_group, status_check_datatype);
                } else if (status_world.MPI_TAG == AVG_EARTH_TEMP && status_world.MPI_SOURCE == n - 1) {
                    avg_earth_temp_request request;
                    MPI_Recv(&request, 1, avg_earth_temp_req_datatype, n - 1, AVG_EARTH_TEMP, MPI_COMM_WORLD,
                             &status_world);
                    printf("GS leader got avg earth temp request!\n");
                    MPI_Send(&request, 1, avg_earth_temp_req_datatype, get_st_leader_gs(), AVG_EARTH_TEMP,
                             MPI_COMM_WORLD);
                    printf("GS leader sent to ST leader!\n");
                } else if (status_world.MPI_TAG == AVG_EARTH_TEMP_DONE && status_world.MPI_SOURCE ==
                           get_st_leader_gs()) {
                    avg_earth_temp data;
                    printf("GS leader waiting to receive avg earth temp done from ST leader\n");
                    MPI_Recv(&data, 1, avg_earth_temp_res_datatype, status_world.MPI_SOURCE, AVG_EARTH_TEMP_DONE,
                             MPI_COMM_WORLD,
                             &status_world);
                    printf("GS leader got final avg temp from ST leader. Adding and sending to coordinator\n");
                    add_metric_gs(avg_metrics, data.timestamp, data.avg_temperature);
                    MPI_Send(NULL, 0, MPI_INT, n - 1, AVG_EARTH_TEMP_DONE, MPI_COMM_WORLD);
                } else if (status_world.MPI_TAG == SYNC && status_world.MPI_SOURCE == n - 1) {
                    sync data;
                    MPI_Recv(&data.gs_leader, 1, MPI_INT, n - 1, SYNC, MPI_COMM_WORLD, &status_world);
                    printf("GS %d got sync from COORDINATOR\n", rank);
                    data.gs_leader %= group_size; // get local gs leader rank
                    data.total_checks = get_status_checks_count();

                    if (group_rank != data.gs_leader) {
                        send_check_count_to_leader(rank, &data, group_rank, comm_group, sync_datatype);
                    } else {
                        printf("is leader, not sending further\n");
                        total_checks += get_status_checks_count();
                    }
                }
                //todo:sp add sync done
            }

            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm_group, &probe_gs_flag, &status_gs);

            if (probe_gs_flag) {
                if (status_gs.MPI_TAG == CONNECT) {
                    // got CONNECT from another GS
                    gs_connect received_data;

                    MPI_Recv(&received_data, 1, connect_datatype, MPI_ANY_SOURCE, CONNECT, comm_group, &status_gs);

                    if (status_gs.MPI_SOURCE == received_data.gs_rank % group_size) {
                        // add as child since the event came from the gs_rank source
                        add_neighbor_gs(status_gs.MPI_SOURCE); // add neighbor with group rank
                        MPI_Send((void *) 0, 0, MPI_INT, status_gs.MPI_SOURCE, ACK, comm_group);
                    }
                } else if (status_gs.MPI_TAG == ACK) {
                    // receive ack response from neighbor_gs_rank
                    MPI_Recv((void *) 0, 0, MPI_INT, MPI_ANY_SOURCE, ACK, comm_group,
                             &status_gs);
                    // gs_rank got ACK from neighbor_gs_rank
                    // send ACK back to the coordinator
                    MPI_Send((void *) 0, 0, MPI_INT, n - 1, ACK, MPI_COMM_WORLD);
                } else if (status_gs.MPI_TAG == FIND_MIN_DIST) {
                    status_check data;
                    MPI_Recv(&data, 1, status_check_datatype, MPI_ANY_SOURCE, FIND_MIN_DIST, comm_group, &status_gs);
                    printf("GS %d got find min dist from neighbor\n", rank);
                    get_min_dist_gs(group_rank, data, comm_group, status_check_datatype);
                } else if (status_gs.MPI_TAG == SYNC) {
                    // got sync from a node during traversal
                    sync data;
                    printf("GS %d waiting to receive sync from graph\n", rank);
                    MPI_Recv(&data, 1, sync_datatype, status_gs.MPI_SOURCE, SYNC, comm_group, &status_gs);
                    printf("GS %d got sync FROM NEIGHBOR %d\n", rank, status_gs.MPI_SOURCE + group_size);
                    if (data.gs_leader != group_rank) {
                        send_check_count_to_leader(rank, &data, status_gs.MPI_SOURCE, comm_group, sync_datatype);
                    } else {
                        num_of_sync_replies++;
                        printf("====> GS leader got a reply from %d! Count is %d\n", num_of_sync_replies,
                               status_gs.MPI_SOURCE + group_size);
                        total_checks += data.total_checks;
                    }
                    if (num_of_sync_replies == group_size - 1) {
                        printf("GS Leader gathered %d sync replies\n", num_of_sync_replies);
                    }
                } else if (status_gs.MPI_TAG == TERMINATE_LELECT_GS) {
                    int temp;
                    MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, TERMINATE_LELECT_GS, comm_group, &status_gs);
                } else {
                    printf("GS %d got weird event %d from %d\n", rank, status_gs.MPI_TAG,
                           status_gs.MPI_SOURCE + group_size);
                    //MPI_Recv(NULL, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm_group, &status_gs);
                }
            }
        } while (1);

        printf("gs exited loop\n");
        free_neighbor_gs();
        destroy_metrics_list(avg_metrics);
    }

    printf("process %d waiting at barrier\n", rank);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Type_free(&connect_datatype);
    MPI_Type_free(&add_status_datatype);
    MPI_Type_free(&coords_datatype);
    MPI_Type_free(&st_add_coords_datatype);
    MPI_Type_free(&gs_add_coords_datatype);
    MPI_Type_free(&st_add_metric_datatype);
    MPI_Type_free(&st_lelect_probe_datatype);
    MPI_Type_free(&st_lelect_reply_datatype);
    MPI_Type_free(&status_check_datatype);
    MPI_Type_free(&stat_check_gs_datatype);
    MPI_Type_free(&avg_earth_temp_req_datatype);
    MPI_Type_free(&avg_earth_temp_res_datatype);
    MPI_Type_free(&sync_datatype);
    MPI_Comm_free(&comm_group);
    MPI_Finalize();
    return 0;
}
