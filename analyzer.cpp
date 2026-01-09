#include "trip_analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <cctype>
#include <regex>

// Constructor
TripAnalyzer::TripAnalyzer() 
    : totalProcessedRecords(0), 
      skippedRecords(0), 
      maxZoneCount(0), 
      maxZoneHourCount(0) {
    // Reserve memory for better performance
    zoneCounts.reserve(100000);
    zoneHourCounts.reserve(1000000);
}

// Main file processing function
bool TripAnalyzer::processFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'\n";
        return false;
    }
    
    // Check if file is empty
    if (file.peek() == std::ifstream::traits_type::eof()) {
        std::cout << "File is empty. No data to process.\n";
        return true;
    }
    
    std::string line;
    int lineNumber = 0;
    
    // Read header (and skip it)
    std::getline(file, line);
    lineNumber++;
    
    std::cout << "Processing trips...\n";
    
    auto processStart = std::chrono::high_resolution_clock::now();
    
    while (std::getline(file, line)) {
        lineNumber++;
        totalProcessedRecords++;
        
        // Skip empty lines
        if (line.empty()) {
            skippedRecords++;
            continue;
        }
        
        TripRecord record;
        if (parseCSVLine(line, record)) {
            // Extract hour from datetime
            int hour = extractHourFromDateTime(record.datetime);
            if (hour >= 0 && hour <= 23) {
                updateCounts(record.pickupZone, hour);
            } else {
                skippedRecords++;
            }
        } else {
            skippedRecords++;
        }
        
        // Progress indicator for large files
        if (lineNumber % 100000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - processStart);
            std::cout << "Processed " << lineNumber << " records in " << elapsed.count() << " ms...\n";
        }
    }
    
    file.close();
    
    auto processEnd = std::chrono::high_resolution_clock::now();
    auto processDuration = std::chrono::duration_cast<std::chrono::milliseconds>(processEnd - processStart);
    
    std::cout << "\nProcessing complete!\n";
    std::cout << "Processing time: " << processDuration.count() << " ms\n";
    std::cout << "Total records processed: " << totalProcessedRecords << "\n";
    std::cout << "Valid records: " << (totalProcessedRecords - skippedRecords) << "\n";
    std::cout << "Skipped records (dirty data): " << skippedRecords << "\n";
    std::cout << "Unique zones found: " << zoneCounts.size() << "\n";
    std::cout << "Unique zone-hour slots: " << zoneHourCounts.size() << "\n";
    std::cout << "Most trips in a zone: " << maxZoneCount << "\n";
    std::cout << "Most trips in a slot: " << maxZoneHourCount << "\n";
    
    return true;
}

// New method to ingest file
void TripAnalyzer::ingestFile(const std::string& filename) {
    processFile(filename);
}

// Process single line (for testing)
bool TripAnalyzer::processLine(const std::string& line) {
    TripRecord record;
    if (parseCSVLine(line, record)) {
        int hour = extractHourFromDateTime(record.datetime);
        if (hour >= 0 && hour <= 23) {
            updateCounts(record.pickupZone, hour);
            return true;
        }
    }
    return false;
}

// New method: topZones
std::vector<ZoneCount> TripAnalyzer::topZones(int n) const {
    auto topItems = getTopN(zoneCounts, n, false);
    std::vector<ZoneCount> result;
    result.reserve(topItems.size());
    
    for (const auto& item : topItems) {
        result.push_back({item.first, item.second});
    }
    
    return result;
}

// New method: topBusySlots
std::vector<SlotCount> TripAnalyzer::topBusySlots(int n) const {
    auto topItems = getTopN(zoneHourCounts, n, true);
    std::vector<SlotCount> result;
    result.reserve(topItems.size());
    
    for (const auto& item : topItems) {
        size_t dashPos = item.first.find('-');
        std::string zoneID = item.first.substr(0, dashPos);
        int hour = std::stoi(item.first.substr(dashPos + 1));
        result.push_back({zoneID, hour, item.second});
    }
    
    return result;
}

// Parse CSV line
bool TripAnalyzer::parseCSVLine(const std::string& line, TripRecord& record) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
        token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // Remove quotes if present
        if (token.length() >= 2 && token.front() == '"' && token.back() == '"') {
            token = token.substr(1, token.length() - 2);
        }
        
        tokens.push_back(token);
    }
    
    // Check if we have at least 6 columns
    if (tokens.size() < 6) {
        return false;
    }
    
    // Validate TripID (should be numeric)
    if (tokens[0].empty()) {
        return false;
    }
    
    // Assign values
    record.tripID = tokens[0];
    record.pickupZone = tokens[1];
    record.dropoffZone = tokens[2];
    record.datetime = tokens[3];
    
    // Validate and parse distance
    try {
        record.distance = std::stod(tokens[4]);
        if (record.distance < 0) return false;
    } catch (...) {
        return false;
    }
    
    // Validate and parse fare
    try {
        record.fare = std::stod(tokens[5]);
        if (record.fare < 0) return false;
    } catch (...) {
        return false;
    }
    
    // Additional validation: check datetime format
    if (record.datetime.length() < 16) {
        return false;
    }
    
    return true;
}

// Extract hour from datetime string
int TripAnalyzer::extractHourFromDateTime(const std::string& datetimeStr) {
    // Expected format: "2023-01-10 08:42"
    
    if (datetimeStr.length() < 16) {
        return -1;
    }
    
    // Find space between date and time
    size_t spacePos = datetimeStr.find(' ');
    if (spacePos == std::string::npos) {
        return -1;
    }
    
    std::string timePart = datetimeStr.substr(spacePos + 1);
    
    // Check time format: HH:MM
    if (timePart.length() < 5) {
        return -1;
    }
    
    // Extract hour
    std::string hourStr = timePart.substr(0, 2);
    
    // Validate hour is numeric
    for (char c : hourStr) {
        if (!std::isdigit(c)) {
            return -1;
        }
    }
    
    int hour = std::stoi(hourStr);
    
    // Validate hour range
    if (hour < 0 || hour > 23) {
        return -1;
    }
    
    // Extract minutes for validation
    std::string minuteStr = timePart.substr(3, 2);
    for (char c : minuteStr) {
        if (!std::isdigit(c)) {
            return -1;
        }
    }
    
    int minute = std::stoi(minuteStr);
    if (minute < 0 || minute > 59) {
        return -1;
    }
    
    // Handle 23:59 → hour 23 (as per requirements)
    if (hour == 23 && minute == 59) {
        return 23;
    }
    
    return hour;
}

// Update counts
void TripAnalyzer::updateCounts(const std::string& zoneID, int hour) {
    // Update zone counts
    zoneCounts[zoneID]++;
    if (zoneCounts[zoneID] > maxZoneCount) {
        maxZoneCount = zoneCounts[zoneID];
    }
    
    // Create key for zone-hour combination
    std::stringstream keyStream;
    keyStream << zoneID << "-" << std::setw(2) << std::setfill('0') << hour;
    std::string key = keyStream.str();
    
    // Update zone-hour counts
    zoneHourCounts[key]++;
    if (zoneHourCounts[key] > maxZoneHourCount) {
        maxZoneHourCount = zoneHourCounts[key];
    }
}

// Add test record directly
void TripAnalyzer::addTestRecord(const std::string& zoneID, int hour, int count) {
    for (int i = 0; i < count; i++) {
        updateCounts(zoneID, hour);
        totalProcessedRecords++;
    }
}

// Get top N items with proper tie-breaking
std::vector<std::pair<std::string, long long>> TripAnalyzer::getTopN(
    const std::unordered_map<std::string, long long>& data, 
    int n, 
    bool isZoneHour) const {
    
    std::vector<std::pair<std::string, long long>> items;
    items.reserve(data.size());
    
    // Copy to vector
    for (const auto& pair : data) {
        items.push_back(pair);
    }
    
    // Custom comparator for sorting
    auto comparator = [isZoneHour](const std::pair<std::string, long long>& a, 
                                   const std::pair<std::string, long long>& b) {
        // Primary: count (descending)
        if (a.second != b.second) {
            return a.second > b.second;
        }
        
        // Secondary: key (ascending) with case sensitivity
        // For zone-hour keys like "ZONE001-08", sort by zone then hour
        if (isZoneHour) {
            size_t dashPosA = a.first.find('-');
            size_t dashPosB = b.first.find('-');
            
            if (dashPosA != std::string::npos && dashPosB != std::string::npos) {
                std::string zoneA = a.first.substr(0, dashPosA);
                std::string zoneB = b.first.substr(0, dashPosB);
                
                if (zoneA != zoneB) {
                    return zoneA < zoneB;
                }
                
                // Same zone, compare hours
                int hourA = std::stoi(a.first.substr(dashPosA + 1));
                int hourB = std::stoi(b.first.substr(dashPosB + 1));
                return hourA < hourB;
            }
        }
        
        // For zone-only: case-sensitive alphabetical
        return a.first < b.first;
    };
    
    // Use partial_sort for efficiency - O(n + k log k) where k=10
    if (items.size() > n) {
        std::partial_sort(items.begin(), 
                         items.begin() + n, 
                         items.end(), 
                         comparator);
        items.resize(n);
    } else {
        std::sort(items.begin(), items.end(), comparator);
    }
    
    return items;
}

// Display top zones
void TripAnalyzer::displayTopZones() const {
    auto topZones = getTopN(zoneCounts, 10, false);
    
    std::cout << "\n" << std::string(40, '=') << "\n";
    std::cout << "TOP 10 PICKUP ZONES\n";
    std::cout << std::string(40, '=') << "\n";
    
    if (topZones.empty()) {
        std::cout << "No zone data available.\n";
        return;
    }
    
    std::cout << std::left << std::setw(6) << "Rank" 
              << std::setw(15) << "Zone ID" 
              << std::setw(15) << "Trip Count" 
              << "Visualization\n";
    std::cout << std::string(60, '-') << "\n";
    
    int rank = 1;
    long long maxCount = topZones.empty() ? 1 : topZones[0].second;
    
    for (const auto& zone : topZones) {
        std::cout << std::left << std::setw(6) << (rank++) 
                  << std::setw(15) << zone.first 
                  << std::setw(15) << zone.second;
        
        // ASCII bar chart
        int barLength = maxCount > 0 ? (zone.second * 40) / maxCount : 0;
        if (barLength < 1 && zone.second > 0) barLength = 1;
        
        std::cout << "[";
        for (int i = 0; i < barLength; i++) {
            std::cout << "█";
        }
        
        // Add tie indicator if needed
        if (rank > 2 && zone.second == topZones[rank-3].second) {
            std::cout << " (tie)";
        }
        
        std::cout << "]\n";
    }
}

// Display top slots
void TripAnalyzer::displayTopSlots() const {
    auto topSlots = getTopN(zoneHourCounts, 10, true);
    
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "TOP 10 BUSY SLOTS (Zone + Hour)\n";
    std::cout << std::string(50, '=') << "\n";
    
    if (topSlots.empty()) {
        std::cout << "No zone-hour data available.\n";
        return;
    }
    
    std::cout << std::left << std::setw(6) << "Rank" 
              << std::setw(20) << "Zone-Hour Slot" 
              << std::setw(20) << "Time" 
              << std::setw(15) << "Trip Count" 
              << "Visualization\n";
    std::cout << std::string(80, '-') << "\n";
    
    int rank = 1;
    long long maxCount = topSlots.empty() ? 1 : topSlots[0].second;
    
    for (const auto& slot : topSlots) {
        // Parse zone and hour from key
        size_t dashPos = slot.first.find('-');
        std::string zoneID = slot.first.substr(0, dashPos);
        std::string hourStr = slot.first.substr(dashPos + 1);
        int hour = std::stoi(hourStr);
        
        // Format time display
        std::stringstream timeDisplay;
        timeDisplay << std::setw(2) << std::setfill('0') << hour << ":00";
        if (hour < 12) {
            timeDisplay << " AM";
        } else if (hour == 12) {
            timeDisplay << " PM";
        } else {
            timeDisplay << " PM";
        }
        
        std::cout << std::left << std::setw(6) << (rank++) 
                  << std::setw(20) << slot.first 
                  << std::setw(20) << timeDisplay.str()
                  << std::setw(15) << slot.second;
        
        // ASCII bar chart
        int barLength = maxCount > 0 ? (slot.second * 40) / maxCount : 0;
        if (barLength < 1 && slot.second > 0) barLength = 1;
        
        std::cout << "[";
        for (int i = 0; i < barLength; i++) {
            std::cout << "█";
        }
        
        // Add tie indicator if needed
        if (rank > 2 && slot.second == topSlots[rank-3].second) {
            std::cout << " (tie)";
        }
        
        std::cout << "]\n";
    }
}

// Test functions
bool TripAnalyzer::testTieBreaking() const {
    // Test tie-breaking logic
    std::unordered_map<std::string, long long> testData = {
        {"ZONE002", 5},
        {"ZONE001", 5},  // Same count, should come first alphabetically
        {"ZONE003", 3}
    };
    
    auto top = getTopN(testData, 2, false);
    
    if (top.size() >= 2 && top[0].first == "ZONE001" && top[1].first == "ZONE002") {
        return true;
    }
    return false;
}

bool TripAnalyzer::testCaseSensitivity() const {
    // Test that zone01 and ZONE01 are treated as different
    std::unordered_map<std::string, long long> testData = {
        {"zone01", 5},
        {"ZONE01", 3},
        {"ZONE02", 4}
    };
    
    auto top = getTopN(testData, 3, false);
    
    // Both should appear separately
    bool foundLower = false, foundUpper = false;
    for (const auto& item : top) {
        if (item.first == "zone01") foundLower = true;
        if (item.first == "ZONE01") foundUpper = true;
    }
    
    return foundLower && foundUpper;
}

bool TripAnalyzer::testBoundaryHours() const {
    // Create test analyzer
    TripAnalyzer testAnalyzer;
    
    // Test 00:00 should give hour 0
    testAnalyzer.processLine("1,ZONE001,ZONE002,2023-01-01 00:00,5.5,75.0");
    
    // Test 23:59 should give hour 23
    testAnalyzer.processLine("2,ZONE001,ZONE002,2023-01-01 23:59,5.5,75.0");
    
    // Check if we have the right hour slots
    bool hasHour0 = false;
    bool hasHour23 = false;
    
    for (const auto& item : testAnalyzer.getZoneHourCounts()) {
        size_t dashPos = item.first.find('-');
        if (dashPos != std::string::npos) {
            std::string hourStr = item.first.substr(dashPos + 1);
            int hour = std::stoi(hourStr);
            if (hour == 0) hasHour0 = true;
            if (hour == 23) hasHour23 = true;
        }
    }
    
    return hasHour0 && hasHour23;
}

void TripAnalyzer::runInternalTests() const {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "INTERNAL TESTS\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "Test 1: Tie-breaking... ";
    if (testTieBreaking()) {
        std::cout << "✓ PASSED\n";
    } else {
        std::cout << "✗ FAILED\n";
    }
    
    std::cout << "Test 2: Case sensitivity... ";
    if (testCaseSensitivity()) {
        std::cout << "✓ PASSED\n";
    } else {
        std::cout << "✗ FAILED\n";
    }
    
    std::cout << "Test 3: Boundary hours... ";
    if (testBoundaryHours()) {
        std::cout << "✓ PASSED\n";
    } else {
        std::cout << "✗ FAILED\n";
    }
}
