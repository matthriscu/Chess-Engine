# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3

# Files
SRC_FILES = $(wildcard *.cpp)
OBJ_FILES = $(SRC_FILES:.cpp=.o)
EXE = my_program

# Header files (that may not have a matching .cpp file)
HEADER_FILES = $(wildcard *.hpp)

# Default target
all: $(EXE)

# Linking the final executable
$(EXE): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(EXE)

# Compiling the object files
%.o: %.cpp $(HEADER_FILES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf *.o $(EXE)

.PHONY: all clean

