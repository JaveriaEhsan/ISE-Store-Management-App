#ifndef SEASON_H
#define SEASON_H

#include <string>

struct Season
{
    std::string startDate;       // dd-MM-yyyy
    std::string endDate;         // dd-MM-yyyy
    double discountPercent = 0;  // the percentage, for example 10
    bool isActive = false;
};

#endif