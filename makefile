# C++ compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.
TARGET = trip_analyzer

# Test executables
TEST_EXES = A1 A2 A3 B1 B2 B3 C1 C2 C3

# Source files
SRCS = trip_analyzer.cpp
MAIN_SRC = main.cpp
TEST_SRCS = $(addsuffix .cpp, $(TEST_EXES))

# Object files
SRC_OBJS = $(SRCS:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Default target
all: $(TARGET) $(TEST_EXES)

# Main executable
$(TARGET): $(MAIN_OBJ) $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJ) $(SRC_OBJS)

# Test executables
A1: A1.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o A1 A1.cpp $(SRC_OBJS)

A2: A2.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o A2 A2.cpp $(SRC_OBJS)

A3: A3.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o A3 A3.cpp $(SRC_OBJS)

B1: B1.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o B1 B1.cpp $(SRC_OBJS)

B2: B2.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o B2 B2.cpp $(SRC_OBJS)

B3: B3.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o B3 B3.cpp $(SRC_OBJS)

C1: C1.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o C1 C1.cpp $(SRC_OBJS)

C2: C2.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o C2 C2.cpp $(SRC_OBJS)

C3: C3.cpp $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) -o C3 C3.cpp $(SRC_OBJS)

# Compile .cpp to .o
%.o: %.cpp trip_analyzer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(TARGET) $(TEST_EXES) test_*.csv

# Run all tests
test: $(TEST_EXES)
	@echo "Running all tests..."
	@for test in $(TEST_EXES); do \
		echo -n "$$test: "; \
		./$$test; \
	done

# Phony targets
.PHONY: all clean test
