#ifndef SEASON_H
#define SEASON_H

#include <string>
using namespace std;

struct Season
{
    string startDate;            // "2026-11-21"
    string endDate;              // "2026-11-30"
    double discountPercent = 0;  // 10 means 10%
    bool isActive = false;       
};

#endif