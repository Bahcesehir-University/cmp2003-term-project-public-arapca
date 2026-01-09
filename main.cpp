#include <iostream>
#include <chrono>
#include "trip_analyzer.h"

// Function to run specific tests
void runTestCases() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "RUNNING TEST CASES\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    // Test 1: Empty File
    {
        TripAnalyzer analyzer;
        std::cout << "Test 1: Empty File... ";
        if (analyzer.processFile("test_empty.csv")) {
            std::cout << "✓ PASSED\n";
        } else {
            std::cout << "✗ FAILED\n";
        }
    }
    
    // Test 2: Dirty Data
    {
        TripAnalyzer analyzer;
        std::cout << "Test 2: Dirty Data Handling... ";
        if (analyzer.processFile("test_dirty.csv")) {
            std::cout << "✓ PASSED (Skipped invalid records)\n";
        } else {
            std::cout << "✗ FAILED\n";
        }
    }
    
    // Add more tests as needed...
}

int main(int argc, char* argv[]) {
    // Check if running in test mode
    if (argc > 1 && std::string(argv[1]) == "--test") {
        runTestCases();
        return 0;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "==========================================\n";
    std::cout << "   City RideShare Trip Analyzer\n";
    std::cout << "==========================================\n\n";
    
    TripAnalyzer analyzer;
    
    // Process the trips file
    bool success = analyzer.processFile("Trips.csv");
    
    if (!success) {
        std::cerr << "Failed to process Trips.csv\n";
        return 1;
    }
    
    // Display results
    analyzer.displayTopZones();
    std::cout << "\n";
    analyzer.displayTopSlots();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n==========================================\n";
    std::cout << "Total execution time: " << duration.count() << " ms\n";
    std::cout << "==========================================\n";
    
    return 0;
}
