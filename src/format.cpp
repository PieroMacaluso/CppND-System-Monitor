#include <string>
#include <ctime>
#include "format.h"

using std::string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    std::time_t timeElapsed = (std::time_t) seconds;
    char buffer[10];
    std::strftime(buffer, 10, "%T", std::gmtime(&timeElapsed));
    return string(buffer);
}