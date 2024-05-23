# Satellite to Ground Station Distributed System
A distributed satellite
communication system, where ground stations coordinate with satellites to exchange data
and execute commands. More specifically, satellites collect information that ground stations
are interested in. Ground stations are also interested in the proper operation of each
satellite.
This project is implemented in C using the Message Passing Interface
(MPI).

## Compilation
Simply run `make all` to compile the program or `make clean` to
remove all generated files.

## Running
You can run the program using `mpirun -np <N+1> ./stg_system <N> <path_to_input>.txt [-d]`
where `<N>` is the number of processes (minus the coordinator), `<path_to_input>` is the file
containing the commands and `-d` is an optional flag that displays debug print statements.

## System & Algorithms Description

The system consists of a Coordinator type process, of type processesSatellite(satellites) and
from type processesGround Station(terrestrial stations).
Purpose ofof Satellite type processes is monitoring the temperature of planet earth. In
addition to the temperature measurement, each Satellite process stores a value (a
percentage) that characterizes its correct operation (Status). Finally, it has a position in
space, which is represented through the geographic coordinate system - more on this in
Section 5.
TheGround Station type processes run on devices on the ground that communicate with the
satellites. More specifically, they are interested in the data collected by the satellites, as well
as the state of their proper operation. Like satellites, ground stations have a place on Earth.
This position is given by the geographic coordinate system.
In summary, each Satellite process is characterized by the following parameters, its
identifier (id) (mpi_rank), its coordinates, even though they are stored in a local table,
coordinates[3] (latitude, longitude, altitude), its process (see Section 5), its health status
(status), and a table of temperature measurement pairs (accompanied by the time when the
measurement was made). Similarly, each Ground Station process is characterized by the
following parameters, its identifier (id) (mpi_rank), its coordinates. For the sake of simplicity,
we consider that the coordinates array of each such process stores three components (with
elevation = 0). A Ground Station process also stores an array of pairs of the form
<temperature measurement, time the measurement was taken>, where each pair 
characterizes a measurement the ground station received from a satellite.
Based on the above, the system should be configured as follows:
❖ They existN+1processes in the system, where theNis given as a parameter to
command line at startup and is an even-positive number.
❖ Assume that there is a process with an identifierNwhich plays its part coordinator
❖ Press processesSatellite isY/2in the crowd and have IDs
[0, (N/2)-1]. Ground stations processes are alsoY/2in the crowd and they have
identifiers[N/2, N-1]. For example if N = 16, there are 17 processes in the system. 
Satellite-type processes will have identifiers 0,1 .., 7, while ground station processes 
have identifiers 8, ..., 15. Process 16 is the coordinator.
❖ The coordinator process (coordinator) reads a testfile that will contain the description 
of various events that the rest of the processes must simulate and sends 
corresponding messages to the appropriate processes to make this happen. These 
events are described in Section 3.
❖ Press processesGround Stations request the execution of two types of requests, and 
Satellite-type processes serve them. The first type of requests concerns 
measurements of the Earth's temperature, while the second type concerns the 
proper operation of a Satellite-type process (more information below).
❖ Every processsatellite maintains a structure with temperature measurements and 
uses the measurements stored in this structure to answer the requests of Ground 
Station type processes. The elements of the structure are determined by 
ADD_METRIC type events, which are sent to them by the coordinator (and
simulate the measurement process). (These events are described in detail in Section 
3.)
❖ Every processGround Station maintains a corresponding structure that stores the 
average temperature received from the satellite processes for a certain timestamp. 
This structure stores temperatures for several moments (according to testfile 
events). More in Section 3.3.
❖ Press processesSatellites are connected to each other forming a ring (as shown in 
Figure 1). Specifically, the ring is created as follows. Process j, 1 <= j <= N/2, has as 
neighboring processes in the ring the processes with identifier (j+1) %N/2 and (j-1), 
while process 0 has processes 1 and N/2-1, as its neighbors in the ring (see Figure 1).
❖ Press processesGround Stations form a tree, with no given root (see Figure 2 for an 
example). The way these processes are interconnected is determined by the 
coordinator, which reads and processes the testfile during system startup, as 
described in detail in Section 3.1.
❖ One of the type processesSatellite should be elected leader as described in Section 
2.1 and 3.2. Likewise, one of the Ground Station processes will be elected leader, as 
described in Section 2.2 and 3.2.



