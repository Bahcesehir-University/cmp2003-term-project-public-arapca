#include "trip_analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <iomanip>
#include <random>
#include <chrono>

// Constructor
TripAnalyzer::TripAnalyzer() : totalRecords(0), validRecords(0), skippedRecords(0) {}

// Main ingestion function
void TripAnalyzer::ingestFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'\n";
        return;
    }
    
    std::string line;
    
    // Skip header line
    if (!std::getline(file, line)) {
        return; // Empty file
    }
    
    // Process each line
    while (std::getline(file, line)) {
        totalRecords++;
        
        std::string zoneID;
        int hour;
        
        if (parseCSVLine(line, zoneID, hour)) {
            validRecords++;
            
            // Update zone counts
            zoneCounts[zoneID]++;
            
            // Update zone-hour counts
            zoneHourCounts[zoneID][hour]++;
        } else {
            skippedRecords++;
        }
    }
    
    file.close();
}

// Parse a CSV line and extract zone and hour
bool TripAnalyzer::parseCSVLine(const std::string& line, std::string& zoneID, int& hour) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        size_t start = token.find_first_not_of(" \t\r\n");
        size_t end = token.find_last_not_of(" \t\r\n");
        
        if (start == std::string::npos) {
            tokens.push_back("");
        } else {
            tokens.push_back(token.substr(start, end - start + 1));
        }
    }
    
    // Need at least TripID, PickupZoneID, and PickupTime
    if (tokens.size() < 3) {
        return false;
    }
    
    // Check for empty zone ID
    if (tokens[1].empty()) {
        return false;
    }
    
    zoneID = tokens[1];
    
    // Extract hour from PickupTime
    hour = extractHour(tokens[2]);
    
    return hour >= 0 && hour <= 23;
}

// Extract hour from datetime string (YYYY-MM-DD HH:MM)
int TripAnalyzer::extractHour(const std::string& datetime) {
    // Expected format: "YYYY-MM-DD HH:MM"
    if (datetime.length() < 16) {
        return -1;
    }
    
    // Find space between date and time
    size_t spacePos = datetime.find(' ');
    if (spacePos == std::string::npos || spacePos + 3 >= datetime.length()) {
        return -1;
    }
    
    // Extract hour part (HH from HH:MM)
    std::string hourStr = datetime.substr(spacePos + 1, 2);
    
    // Validate hour digits
    for (char c : hourStr) {
        if (!std::isdigit(c)) {
            return -1;
        }
    }
    
    try {
        int hour = std::stoi(hourStr);
        if (hour < 0 || hour > 23) {
            return -1;
        }
        return hour;
    } catch (...) {
        return -1;
    }
}

// Get top k zones
std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;
    result.reserve(zoneCounts.size());
    
    // Convert map to vector
    for (const auto& pair : zoneCounts) {
        result.push_back({pair.first, pair.second});
    }
    
    // Sort using operator<
    std::sort(result.begin(), result.end());
    
    // Return top k (or all if less than k)
    if (k > 0 && static_cast<size_t>(k) < result.size()) {
        result.resize(k);
    }
    
    return result;
}

// Get top k busy slots
std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    
    // Convert nested maps to vector
    for (const auto& zonePair : zoneHourCounts) {
        const std::string& zone = zonePair.first;
        for (const auto& hourPair : zonePair.second) {
            result.push_back({zone, hourPair.first, hourPair.second});
        }
    }
    
    // Sort using operator<
    std::sort(result.begin(), result.end());
    
    // Return top k (or all if less than k)
    if (k > 0 && static_cast<size_t>(k) < result.size()) {
        result.resize(k);
    }
    
    return result;
}

// Clear all data
void TripAnalyzer::clear() {
    zoneCounts.clear();
    zoneHourCounts.clear();
    totalRecords = 0;
    validRecords = 0;
    skippedRecords = 0;
}

// Direct manipulation for testing
void TripAnalyzer::addZoneCount(const std::string& zone, int count) {
    zoneCounts[zone] += count;
}

void TripAnalyzer::addZoneHourCount(const std::string& zone, int hour, int count) {
    zoneHourCounts[zone][hour] += count;
}

// Test functions
bool TripAnalyzer::runEmptyFileTest() {
    std::ofstream file("test_empty.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    file.close();
    
    clear();
    ingestFile("test_empty.csv");
    
    bool result = (validRecords == 0 && totalRecords == 0);
    std::remove("test_empty.csv");
    return result;
}

bool TripAnalyzer::runDirtyDataTest() {
    std::ofstream file("test_dirty.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    file << "1,ZONE001,2023-01-01 08:30\n";  // Valid
    file << "2,,2023-01-01 09:30\n";          // Missing zone
    file << "3,ZONE002,invalid-time\n";       // Invalid time
    file << "4,ZONE003,2023-01-01 25:30\n";   // Invalid hour
    file << "5,ZONE004,2023-01-01 12:30\n";   // Valid
    file.close();
    
    clear();
    ingestFile("test_dirty.csv");
    
    bool result = (validRecords == 2 && skippedRecords == 3);
    std::remove("test_dirty.csv");
    return result;
}

bool TripAnalyzer::runBoundaryHoursTest() {
    std::ofstream file("test_boundary.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    file << "1,ZONE001,2023-01-01 00:00\n";  // Hour 0
    file << "2,ZONE001,2023-01-01 23:59\n";  // Hour 23
    file << "3,ZONE002,2023-01-01 12:30\n";  // Hour 12
    file.close();
    
    clear();
    ingestFile("test_boundary.csv");
    
    // Check if hours 0 and 23 are correctly parsed
    bool foundHour0 = false;
    bool foundHour23 = false;
    
    for (const auto& zonePair : zoneHourCounts) {
        for (const auto& hourPair : zonePair.second) {
            if (hourPair.first == 0) foundHour0 = true;
            if (hourPair.first == 23) foundHour23 = true;
        }
    }
    
    bool result = (foundHour0 && foundHour23 && validRecords == 3);
    std::remove("test_boundary.csv");
    return result;
}

bool TripAnalyzer::runTieBreakerTest() {
    std::ofstream file("test_tie.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    file << "1,ZONE_B,2023-01-01 08:30\n";
    file << "2,ZONE_B,2023-01-01 09:30\n";
    file << "3,ZONE_A,2023-01-01 10:30\n";
    file << "4,ZONE_A,2023-01-01 11:30\n";
    file << "5,ZONE_C,2023-01-01 12:30\n";
    file.close();
    
    clear();
    ingestFile("test_tie.csv");
    
    auto zones = topZones(3);
    
    bool result = (zones.size() >= 2 && 
                   zones[0].zone == "ZONE_A" && zones[0].count == 2 &&
                   zones[1].zone == "ZONE_B" && zones[1].count == 2);
    
    std::remove("test_tie.csv");
    return result;
}

bool TripAnalyzer::runSingleHitTest() {
    std::ofstream file("test_single.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    for (int i = 1; i <= 15; i++) {
        file << i << ",ZONE" << std::setw(3) << std::setfill('0') << i 
             << ",2023-01-01 08:30\n";
    }
    file.close();
    
    clear();
    ingestFile("test_single.csv");
    
    auto zones = topZones(10);
    
    bool result = (zones.size() == 10);
    if (result) {
        for (int i = 0; i < 10; i++) {
            std::string expected = "ZONE" + std::string(3 - std::to_string(i+1).length(), '0') + std::to_string(i+1);
            if (zones[i].zone != expected) {
                result = false;
                break;
            }
        }
    }
    
    std::remove("test_single.csv");
    return result;
}

bool TripAnalyzer::runCaseSensitivityTest() {
    std::ofstream file("test_case.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    file << "1,zoneA,2023-01-01 08:30\n";
    file << "2,ZONEA,2023-01-01 09:30\n";
    file << "3,ZoneA,2023-01-01 10:30\n";
    file.close();
    
    clear();
    ingestFile("test_case.csv");
    
    auto zones = topZones(10);
    
    bool result = (zones.size() == 3); // All should be different
    
    std::remove("test_case.csv");
    return result;
}

bool TripAnalyzer::runHighCollisionTest() {
    std::ofstream file("test_collision.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    
    // Create 1000 records with 70% ZONE001, 30% ZONE002
    for (int i = 1; i <= 1000; i++) {
        std::string zone = (i % 10 < 7) ? "ZONE001" : "ZONE002";
        int hour = 8 + (i % 10);
        file << i << "," << zone << ",2023-01-01 "
             << std::setw(2) << std::setfill('0') << hour << ":30\n";
    }
    file.close();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    clear();
    ingestFile("test_collision.csv");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    auto zones = topZones(2);
    bool correctCounts = (zones.size() >= 2 && 
                         zones[0].zone == "ZONE001" && zones[0].count == 700 &&
                         zones[1].zone == "ZONE002" && zones[1].count == 300);
    
    bool fastEnough = (duration.count() < 100); // Should process in < 100ms
    
    std::remove("test_collision.csv");
    return correctCounts && fastEnough;
}

bool TripAnalyzer::runHighCardinalityTest() {
    std::ofstream file("test_cardinality.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    
    for (int i = 1; i <= 1000; i++) {
        file << i << ",ZONE" << std::setw(4) << std::setfill('0') << i 
             << ",2023-01-01 08:30\n";
    }
    file.close();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    clear();
    ingestFile("test_cardinality.csv");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    bool correctCount = (zoneCounts.size() == 1000);
    bool fastEnough = (duration.count() < 200); // Should process in < 200ms
    
    std::remove("test_cardinality.csv");
    return correctCount && fastEnough;
}

bool TripAnalyzer::runVolumeTest() {
    std::ofstream file("test_volume.csv");
    file << "TripID,PickupZoneID,PickupTime\n";
    
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<int> zoneDist(1, 100);
    std::uniform_int_distribution<int> hourDist(0, 23);
    
    for (int i = 1; i <= 10000; i++) {
        int zoneNum = zoneDist(rng);
        int hour = hourDist(rng);
        
        file << i << ",ZONE" << std::setw(3) << std::setfill('0') << zoneNum 
             << ",2023-01-01 " << std::setw(2) << std::setfill('0') << hour << ":30\n";
    }
    file.close();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    clear();
    ingestFile("test_volume.csv");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    bool correctCount = (validRecords == 10000);
    bool fastEnough = (duration.count() < 500); // Should process in < 500ms
    
    std::remove("test_volume.csv");
    return correctCount && fastEnough;
}
