cmake_minimum_required(VERSION 3.28)
project(STG_Communication_System C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(mpi)

add_executable(STG_Communication_System
        main.c
        common/constants.h
        common/event_payloads.h
        election/leader_elect_manager.h
        election/leader_elect_manager.c
        satellite/satellite.h
        satellite/satellite.c
        common/temp_metrics.h
        common/temp_metrics.c
        ground_station/ground_station.c
        ground_station/ground_station.h
        common/mpi_datatypes.c
        common/mpi_datatypes.h
        coordinator/coordinator.c
        coordinator/coordinator.h
        common/utils.h
        common/debug_utils.c
        common/debug_utils.h
        common/debug_utils.h)