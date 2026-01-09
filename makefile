# C++ compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.
TARGET = trip_analyzer
TEST_TARGET = run_tests

# Source files
SRCS = trip_analyzer.cpp
MAIN_SRC = main.cpp $(SRCS)
TEST_SRCS = test_main.cpp $(SRCS)

# Object files
MAIN_OBJS = $(MAIN_SRC:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Default target
all: $(TARGET) $(TEST_TARGET)

# Main executable
$(TARGET): $(MAIN_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJS)

# Test executable
$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) test_*.csv

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Run main program
run: $(TARGET)
	./$(TARGET)

# Run main in test mode
run-test: $(TARGET)
	./$(TARGET) --test

# Phony targets
.PHONY: all clean test run run-test
