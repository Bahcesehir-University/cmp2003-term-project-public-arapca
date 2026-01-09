#include "trip_analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <iomanip>

// Constructor
TripAnalyzer::TripAnalyzer() : totalRecords(0), validRecords(0) {}

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

// Clear all data (for testing)
void TripAnalyzer::clear() {
    zoneCounts.clear();
    zoneHourCounts.clear();
    totalRecords = 0;
    validRecords = 0;
}
