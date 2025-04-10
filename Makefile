CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3

# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))
EXE = engine

HEADER_FILES = $(wildcard $(SRC_DIR)/*.hpp)

all: $(EXE)

run: $(EXE)
	./$(EXE)

# Link all object files into the executable
$(EXE): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(EXE) $(CXXFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADER_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXE)

.PHONY: all clean run

