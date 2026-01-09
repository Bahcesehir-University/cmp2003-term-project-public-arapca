#include <iostream>
#include <chrono>
#include "trip_analyzer.h"

int main(int argc, char* argv[]) {
    std::cout << "==========================================\n";
    std::cout << "   City RideShare Trip Analyzer\n";
    std::cout << "==========================================\n\n";
    
    TripAnalyzer analyzer;
    
    // Process the trips file
    std::cout << "Processing trips file...\n";
    analyzer.ingestFile("SmallTrips.csv");
    
    // Display results
    std::cout << "\n=== Top 10 Pickup Zones ===\n";
    auto topZones = analyzer.topZones(10);
    int rank = 1;
    for (const auto& zone : topZones) {
        std::cout << rank++ << ". " << zone.zone << " - " << zone.count << " trips\n";
    }
    
    std::cout << "\n=== Top 10 Busy Slots ===\n";
    auto topSlots = analyzer.topBusySlots(10);
    rank = 1;
    for (const auto& slot : topSlots) {
        std::cout << rank++ << ". Zone " << slot.zone 
                  << " at " << slot.hour << ":00 - " 
                  << slot.count << " trips\n";
    }
    
    return 0;
}
