#ifndef DATEUTIL_H
#define DATEUTIL_H

#include <string>

// Dates are written dd-MM-yyyy, times HH:mm.
std::string todayDate();
std::string nowTime();

// "21-11-2026" -> "20261121", so dates can be compared as text. "" if not a date.
std::string dateKey(const std::string& date);

bool parseDate(const std::string& date, int& day, int& month, int& year);

// date + n days (n can be negative). "" if the date is bad.
std::string addDays(const std::string& date, int n);

// days from "from" to "to" (to - from)
bool daysBetween(const std::string& from, const std::string& to, int& days);

#endif