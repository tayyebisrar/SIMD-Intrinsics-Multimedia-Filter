TARGET = filter
FLAGS = -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-deprecated-declarations
CC = g++
SRC_DIR = src
INCLUDE_DIR = include
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/helpers.cpp 
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)
	@echo "Compilation successful: $(TARGET) created."

%.o: %.cpp
	$(CC) $(FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Cleaned up build files."
