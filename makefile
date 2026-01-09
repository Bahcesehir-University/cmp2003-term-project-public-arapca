# C++ compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.
TARGET = trip_analyzer

# Source files
SRCS = trip_analyzer.cpp test_cases.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Main executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test targets (these match what the autograder expects)
A1: $(TARGET)
	./$(TARGET) --test-empty

A2: $(TARGET)
	./$(TARGET) --test-dirty

A3: $(TARGET)
	./$(TARGET) --test-boundary

B1: $(TARGET)
	./$(TARGET) --test-tie

B2: $(TARGET)
	./$(TARGET) --test-single

B3: $(TARGET)
	./$(TARGET) --test-case

C1: $(TARGET)
	./$(TARGET) --test-collision

C2: $(TARGET)
	./$(TARGET) --test-cardinality

C3: $(TARGET)
	./$(TARGET) --test-volume

# Clean build files
clean:
	rm -f *.o $(TARGET) test_*.csv

# Run all tests
test: $(TARGET)
	./$(TARGET) --test-all

# Phony targets
.PHONY: all clean test A1 A2 A3 B1 B2 B3 C1 C2 C3
