#include "trip_analyzer.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>

// Helper to create test files
void createTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    file << "TripID,PickupZoneID,PickupTime\n";
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Test 1: Basic functionality
bool testBasicFunctionality() {
    std::cout << "Test 1: Basic functionality... ";
    
    createTestFile("test_basic.csv", {
        "1,ZONE_A,2024-01-01 08:30",
        "2,ZONE_A,2024-01-01 09:30",
        "3,ZONE_B,2024-01-01 10:30",
        "4,ZONE_C,2024-01-01 11:30",
        "5,ZONE_A,2024-01-01 12:30"
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_basic.csv");
    
    auto zones = analyzer.topZones(10);
    bool result = (zones.size() >= 3) && 
                  (zones[0].zone == "ZONE_A") && (zones[0].count == 3) &&
                  (analyzer.getValidRecords() == 5);
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_basic.csv");
    return result;
}

// Test 2: Empty file
bool testEmptyFile() {
    std::cout << "Test 2: Empty file... ";
    
    createTestFile("test_empty.csv", {});
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_empty.csv");
    
    auto zones = analyzer.topZones(10);
    auto slots = analyzer.topBusySlots(10);
    
    bool result = (zones.empty() && slots.empty() && analyzer.getValidRecords() == 0);
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_empty.csv");
    return result;
}

// Test 3: Malformed data
bool testMalformedData() {
    std::cout << "Test 3: Malformed data... ";
    
    createTestFile("test_malformed.csv", {
        "1,ZONE_A,2024-01-01 08:30",      // Valid
        "2,,2024-01-01 09:30",            // Missing zone
        "3,ZONE_B,invalid-time",          // Invalid time
        "4,ZONE_C,2024-01-01 25:30",      // Invalid hour
        "5,ZONE_A,2024-01-01 12:30"       // Valid
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_malformed.csv");
    
    bool result = (analyzer.getValidRecords() == 2); // Only 2 valid records
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_malformed.csv");
    return result;
}

// Test 4: Tie-breaking
bool testTieBreaking() {
    std::cout << "Test 4: Tie-breaking... ";
    
    createTestFile("test_tie.csv", {
        "1,ZONE_B,2024-01-01 08:30",
        "2,ZONE_A,2024-01-01 09:30",
        "3,ZONE_B,2024-01-01 10:30",
        "4,ZONE_A,2024-01-01 11:30",
        "5,ZONE_C,2024-01-01 12:30"
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_tie.csv");
    
    auto zones = analyzer.topZones(3);
    
    bool result = (zones.size() >= 2) &&
                  (zones[0].zone == "ZONE_A") && (zones[0].count == 2) &&
                  (zones[1].zone == "ZONE_B") && (zones[1].count == 2);
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_tie.csv");
    return result;
}

// Test 5: Case sensitivity
bool testCaseSensitivity() {
    std::cout << "Test 5: Case sensitivity... ";
    
    createTestFile("test_case.csv", {
        "1,zoneA,2024-01-01 08:30",
        "2,ZONEA,2024-01-01 09:30",
        "3,ZoneA,2024-01-01 10:30"
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_case.csv");
    
    auto zones = analyzer.topZones(10);
    
    bool result = (zones.size() == 3); // All should be different
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_case.csv");
    return result;
}

// Test 6: Boundary hours (00:00 and 23:59)
bool testBoundaryHours() {
    std::cout << "Test 6: Boundary hours... ";
    
    createTestFile("test_boundary.csv", {
        "1,ZONE_A,2024-01-01 00:00",
        "2,ZONE_A,2024-01-01 23:59",
        "3,ZONE_A,2024-01-01 12:30"
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_boundary.csv");
    
    auto slots = analyzer.topBusySlots(10);
    
    bool foundHour0 = false;
    bool foundHour23 = false;
    
    for (const auto& slot : slots) {
        if (slot.zone == "ZONE_A" && slot.hour == 0) foundHour0 = true;
        if (slot.zone == "ZONE_A" && slot.hour == 23) foundHour23 = true;
    }
    
    bool result = foundHour0 && foundHour23;
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_boundary.csv");
    return result;
}

// Test 7: Performance with many records
bool testPerformance() {
    std::cout << "Test 7: Performance (1000 records)... ";
    
    std::ofstream file("test_perf.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    
    for (int i = 0; i < 1000; i++) {
        int zoneNum = i % 100; // 100 different zones
        int hour = i % 24; // All hours
        file << i << ",ZONE_" << zoneNum << ",2024-01-01 "
             << std::setw(2) << std::setfill('0') << hour << ":30\n";
    }
    file.close();
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_perf.csv");
    
    bool result = (analyzer.getValidRecords() == 1000);
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_perf.csv");
    return result;
}

// Test 8: Correct slot counting
bool testSlotCounting() {
    std::cout << "Test 8: Slot counting... ";
    
    createTestFile("test_slots.csv", {
        "1,ZONE_A,2024-01-01 08:30",
        "2,ZONE_A,2024-01-01 08:45",
        "3,ZONE_A,2024-01-01 09:30",
        "4,ZONE_B,2024-01-01 08:30",
        "5,ZONE_B,2024-01-01 14:30"
    });
    
    TripAnalyzer analyzer;
    analyzer.ingestFile("test_slots.csv");
    
    auto slots = analyzer.topBusySlots(10);
    
    bool foundZoneA8 = false;
    bool foundZoneA9 = false;
    bool foundZoneB8 = false;
    
    for (const auto& slot : slots) {
        if (slot.zone == "ZONE_A" && slot.hour == 8 && slot.count == 2) foundZoneA8 = true;
        if (slot.zone == "ZONE_A" && slot.hour == 9 && slot.count == 1) foundZoneA9 = true;
        if (slot.zone == "ZONE_B" && slot.hour == 8 && slot.count == 1) foundZoneB8 = true;
    }
    
    bool result = foundZoneA8 && foundZoneA9 && foundZoneB8;
    
    std::cout << (result ? "PASSED" : "FAILED") << "\n";
    std::remove("test_slots.csv");
    return result;
}

// Main test runner
int main() {
    std::cout << "CMP2003 Trip Analyzer - Test Suite\n";
    std::cout << "==================================\n\n";
    
    int passed = 0;
    int total = 8;
    
    passed += testBasicFunctionality() ? 1 : 0;
    passed += testEmptyFile() ? 1 : 0;
    passed += testMalformedData() ? 1 : 0;
    passed += testTieBreaking() ? 1 : 0;
    passed += testCaseSensitivity() ? 1 : 0;
    passed += testBoundaryHours() ? 1 : 0;
    passed += testPerformance() ? 1 : 0;
    passed += testSlotCounting() ? 1 : 0;
    
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "TEST RESULTS: " << passed << "/" << total << " tests passed\n";
    std::cout << std::string(50, '=') << "\n";
    
    return (passed == total) ? 0 : 1;
}
