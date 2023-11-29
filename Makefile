CLIENT_TARGET_EXEC := client
SERVER_TARGET_EXEC := server

BUILD_DIR := ./build
SRC_DIRS := ./src

# Find all the C and C++ files we want to compile
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
CLIENT_SRCS := $(shell find $(SRC_DIRS)/client -name '*.c')
SERVER_SRCS := $(shell find $(SRC_DIRS)/server -name '*.c')
UTIL_SRC := $(SRC_DIRS)/util.c

# Prepends BUILD_DIR and appends .o to every src file
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
CLIENT_OBJS := $(CLIENT_SRCS:%=$(BUILD_DIR)/%.o)
SERVER_OBJS := $(SERVER_SRCS:%=$(BUILD_DIR)/%.o)
UTIL_OBJ := $(BUILD_DIR)/$(SRC_DIRS)/util.c.o

# String substitution (suffix version without %).
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CFLAGS := $(INC_FLAGS) -MMD -MP

all: $(CLIENT_TARGET_EXEC) $(SERVER_TARGET_EXEC)

# The final build step.
$(CLIENT_TARGET_EXEC): $(CLIENT_OBJS) $(UTIL_OBJ)
	$(CC) $(CLIENT_OBJS) $(UTIL_OBJ) -o $@ $(LDFLAGS)

$(SERVER_TARGET_EXEC): $(SERVER_OBJS) $(UTIL_OBJ)
	$(CC) $(SERVER_OBJS) $(UTIL_OBJ) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR) $(CLIENT_TARGET_EXEC) $(SERVER_TARGET_EXEC)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
