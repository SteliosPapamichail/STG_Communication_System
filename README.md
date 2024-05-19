# Satellite to Ground Station Distributed System
Developed by Stelios Papamichail (csd4020)

## Compilation
Simply run `make all` to compile the program or `make clean` to
remove all generated files.

## Running
You can run the program using `mpirun -np <N+1> ./stg_system <N> <path_to_input>.txt [-d]`
where `<N>` is the number of processes (minus the coordinator), `<path_to_input>` is the file
containing the commands and `-d` is an optional flag that displays debug print statements.