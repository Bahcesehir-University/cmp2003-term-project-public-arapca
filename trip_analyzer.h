#ifndef TRIP_ANALYZER_H
#define TRIP_ANALYZER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

// Structure to hold zone count information
struct ZoneCount {
    std::string zone;
    long long count;
    
    // For sorting
    bool operator<(const ZoneCount& other) const {
        if (count != other.count) {
            return count > other.count; // Descending by count
        }
        return zone < other.zone; // Ascending by zone name
    }
};

// Structure to hold slot count information
struct SlotCount {
    std::string zone;
    int hour;
    long long count;
    
    // For sorting
    bool operator<(const SlotCount& other) const {
        if (count != other.count) {
            return count > other.count; // Descending by count
        }
        if (zone != other.zone) {
            return zone < other.zone; // Ascending by zone
        }
        return hour < other.hour; // Ascending by hour
    }
};

class TripAnalyzer {
private:
    // Data stores
    std::unordered_map<std::string, long long> zoneCounts;
    std::unordered_map<std::string, std::unordered_map<int, long long>> zoneHourCounts;
    
    // Statistics
    long long totalRecords;
    long long validRecords;
    
    // Helper functions
    bool parseCSVLine(const std::string& line, std::string& zoneID, int& hour);
    int extractHour(const std::string& datetime);
    
public:
    TripAnalyzer();
    
    // Main interface functions (MUST NOT CHANGE SIGNATURES)
    void ingestFile(const std::string& filename);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;
    
    // Utility functions
    long long getTotalRecords() const { return totalRecords; }
    long long getValidRecords() const { return validRecords; }
    
    // For testing
    void clear();
};

#endif // TRIP_ANALYZER_H
