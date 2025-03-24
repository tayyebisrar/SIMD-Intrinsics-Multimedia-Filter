TARGET = filter
FLAGS = -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-deprecated-declarations
CC = g++
SRC_DIR = src
INCLUDE_DIR = include
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/helpers.cpp $(SRC_DIR)/defaultf.cpp
OBJS = $(SRCS:.cpp=.o)

ARCH := $(shell uname -m)
ifeq ($(findstring 86, $(ARCH)), 86)
	SRCS += $(SRC_DIR)/sse2f.cpp
	FLAGS += -msse2
else ifeq ($(findstring arm, $(ARCH)), arm)
	SRCS += $(SRC_DIR)/neonf.cpp
	FLAGS += -mfpu=neon

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)
	@echo "Compilation successful: $(TARGET) created."

%.o: %.cpp
	$(CC) $(FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Cleaned up build files."
