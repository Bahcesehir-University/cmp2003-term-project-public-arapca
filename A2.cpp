#include "trip_analyzer.h"
#include <iostream>

int main() {
    TripAnalyzer analyzer;
    bool result = analyzer.runDirtyDataTest();
    std::cout << (result ? "PASS" : "FAIL") << std::endl;
    return result ? 0 : 1;
}
