#include "trip_analyzer.h"
#include <iostream>

int main() {
    TripAnalyzer analyzer;
    bool result = analyzer.runBoundaryHoursTest();
    std::cout << (result ? "PASS" : "FAIL") << std::endl;
    return result ? 0 : 1;
}
