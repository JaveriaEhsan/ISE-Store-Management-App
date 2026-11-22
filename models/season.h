#ifndef SEASON_H
#define SEASON_H

#include <string>

struct Season
{
    std::string startDate;       // yyyy-dd-MM
    std::string endDate;         // yyyy-dd-MM
    double discountPercent = 0;  // the percentage, for example 10
    bool isActive = false;
};

#endif