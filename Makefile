TARGET = filter
FLAGS = -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-deprecated-declarations
CC = g++
SRC_DIR = src
INCLUDE_DIR = include
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/helpers.cpp $(SRC_DIR)/defaultf.cpp
OBJ_DIR = obj
ARCH := $(shell uname -m)
use_avx2 ?= 0
ifeq ($(findstring 86, $(ARCH)), 86)
	ifeq ($(use_avx2), 1)
		SRCS+= $(SRC_DIR)/avx2f.cpp
		FLAGS += -mavx2
	else
		SRCS += $(SRC_DIR)/sse2f.cpp
		FLAGS += -msse2
	endif
else ifeq ($(findstring arm, $(ARCH)), arm)
	SRCS += $(SRC_DIR)/neonf.cpp
	FLAGS += -mfpu=neon
endif 

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)


$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)
	@echo "Compilation successful: $(TARGET) created."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f $(OBJ_DIR)/avx2f.o
	@echo "Cleaned up build files."
