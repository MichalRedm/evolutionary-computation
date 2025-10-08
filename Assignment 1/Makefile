# Makefile for the C++ project

# Compiler
CXX = g++

# Compiler flags
# -std=c++11: Use C++11 standard
# -Wall: Enable all warnings
# -Isrc: Include directory for headers
CXXFLAGS = -std=c++11 -Wall -Isrc

# Source directory
SRC_DIR = src

# Build directory
BUILD_DIR = build

# Find all .cpp files in the source directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Generate object file names in the build directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET = $(BUILD_DIR)/main

# On Windows, executables usually have a .exe extension
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
endif

# Default target: build the executable
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile source files into object files
# This creates the build directory if it doesn't exist
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Target to run the executable
run: all
	@echo "Running..."
ifeq ($(OS),Windows_NT)
	$(subst /,\,$(TARGET))
else
	./$(TARGET)
endif

# Target to clean up build artifacts
clean:
	@echo "Cleaning..."
ifeq ($(OS),Windows_NT)
	-if exist $(subst /,\,$(BUILD_DIR)) rmdir /s /q $(subst /,\,$(BUILD_DIR))
else
	-rm -rf $(BUILD_DIR)
endif

# Phony targets
.PHONY: all run clean
