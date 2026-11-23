#ifndef DATEUTIL_H
#define DATEUTIL_H

#include <string>


// Reading accepts both yyyy-MM-dd and dd-MM-yyyy.
std::string todayDate();
std::string nowTime();
std::string makeDate(int day, int month, int year);

// "20261121" for sorting and comparing. "" if the text is not a date.
std::string dateKey(const std::string& date);

bool parseDate(const std::string& date, int& day, int& month, int& year);

// date + n days (n can be negative). "" if the date is bad.
std::string addDays(const std::string& date, int n);

// days from "from" to "to" (to - from)
bool daysBetween(const std::string& from, const std::string& to, int& days);

#endif