TARGET = filter
FLAGS = -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-deprecated-declarations
CC = g++

$(TARGET): main.cpp helpers.cpp
	$(CC) $(FLAGS) -o $(TARGET) main.cpp helpers.cpp
	echo "Compilation successful: $(TARGET) created."

clean:
	rm -f $(TARGET)
	echo "Cleaned up build files."