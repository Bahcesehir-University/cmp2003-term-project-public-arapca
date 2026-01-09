#include "trip_analyzer.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    TripAnalyzer analyzer;
    
    // Handle command line arguments for tests
    if (argc > 1) {
        std::string arg = argv[1];
        
        if (arg == "--test-empty") analyzer.runTest("empty");
        else if (arg == "--test-dirty") analyzer.runTest("dirty");
        else if (arg == "--test-boundary") analyzer.runTest("boundary");
        else if (arg == "--test-tie") analyzer.runTest("tie");
        else if (arg == "--test-single") analyzer.runTest("single");
        else if (arg == "--test-case") analyzer.runTest("case");
        else if (arg == "--test-collision") analyzer.runTest("collision");
        else if (arg == "--test-cardinality") analyzer.runTest("cardinality");
        else if (arg == "--test-volume") analyzer.runTest("volume");
        else if (arg == "--test-all") {
            analyzer.runTest("empty");
            analyzer.runTest("dirty");
            analyzer.runTest("boundary");
            analyzer.runTest("tie");
            analyzer.runTest("single");
            analyzer.runTest("case");
            analyzer.runTest("collision");
            analyzer.runTest("cardinality");
            analyzer.runTest("volume");
        }
    } else {
        // Default behavior: run the actual program
        analyzer.ingestFile("SmallTrips.csv");
        
        std::cout << "=== Top 10 Pickup Zones ===" << std::endl;
        auto zones = analyzer.topZones(10);
        int rank = 1;
        for (const auto& zone : zones) {
            std::cout << rank++ << ". " << zone.zone << " - " << zone.count << " trips" << std::endl;
        }
        
        std::cout << "\n=== Top 10 Busy Slots ===" << std::endl;
        auto slots = analyzer.topBusySlots(10);
        rank = 1;
        for (const auto& slot : slots) {
            std::cout << rank++ << ". Zone " << slot.zone 
                      << " at " << slot.hour << ":00 - " 
                      << slot.count << " trips" << std::endl;
        }
    }
    
    return 0;
}
