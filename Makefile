TARGET = filter
FLAGS = -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-deprecated-declarations
CC = g++
SRC_DIR = src
INCLUDE_DIR = include
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/helpers.cpp $(SRC_DIR)/defaultf.cpp $(SRC_DIR)/sse2f.cpp
OBJS = $(SRCS:.cpp=.o)

# neon flag required for ARMv7-A architecture, unneeded for x86 or ARMv8-A
use_neon ?= 0
ifeq ($(use_neon), 1)
	FLAGS += -mfpu=neon
endif

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)
	@echo "Compilation successful: $(TARGET) created."

%.o: %.cpp
	$(CC) $(FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Cleaned up build files."
