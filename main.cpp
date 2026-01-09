#include "trip_analyzer.h"
#include <iostream>

int main() {
    TripAnalyzer analyzer;
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
    
    return 0;
}
