# Compiler
CC = mpicc

# Directories
COMMON_DIR = common
COORDINATOR_DIR = coordinator
SATELLITE_DIR = satellite
GROUND_STATION_DIR = ground_station
ELECTION_DIR = election

# Source files
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.c)
COORDINATOR_SRCS = $(wildcard $(COORDINATOR_DIR)/*.c)
SATELLITE_SRCS = $(wildcard $(SATELLITE_DIR)/*.c)
GROUND_STATION_SRCS = $(wildcard $(GROUND_STATION_DIR)/*.c)
ROOT_SRCS = main.c

# Object files
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
COORDINATOR_OBJS = $(COORDINATOR_SRCS:.c=.o)
SATELLITE_OBJS = $(SATELLITE_SRCS:.c=.o)
GROUND_STATION_OBJS = $(GROUND_STATION_SRCS:.c=.o)
ELECTION_OBJS = $(ELECTION_SRCS:.c=.o)
ROOT_OBJS = $(ROOT_SRCS:.c=.o)

# Executable name
EXEC = stg_system

# Flags
CFLAGS = -Wall
LDFLAGS = -lm

# Targets
all: $(EXEC)

$(EXEC): $(COMMON_OBJS) $(COORDINATOR_OBJS) $(SATELLITE_OBJS) $(GROUND_STATION_OBJS) $(ROOT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile rules
$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(COORDINATOR_DIR)/%.o: $(COORDINATOR_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(SATELLITE_DIR)/%.o: $(SATELLITE_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(GROUND_STATION_DIR)/%.o: $(GROUND_STATION_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f $(EXEC) $(COMMON_OBJS) $(COORDINATOR_OBJS) $(SATELLITE_OBJS) $(GROUND_STATION_OBJS) $(ROOT_OBJS)