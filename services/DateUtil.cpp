#include "DateUtil.h"
#include <cmath>
#include <cstdio>
#include <ctime>

using namespace std;

// ONE switch for the format the project writes.
// true  = yyyy-MM-dd (2026-11-21)     false = dd-MM-yyyy (21-11-2026)
static const bool WRITE_ISO_DATES = true;

static bool allDigits(const string& s)
{
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

string makeDate(int day, int month, int year)
{
    char buffer[16];
    if (WRITE_ISO_DATES) snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", year, month, day);
    else                 snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d", day, month, year);
    return buffer;
}

string todayDate()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    return makeDate(t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
}

string nowTime()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M", t);
    return buffer;
}

bool parseDate(const string& date, int& day, int& month, int& year)
{
    if (date.size() != 10) return false;

    string d, m, y;
    if (date[4] == '-' && date[7] == '-')          // yyyy-MM-dd
    {
        y = date.substr(0, 4); m = date.substr(5, 2); d = date.substr(8, 2);
    }
    else if (date[2] == '-' && date[5] == '-')     // dd-MM-yyyy
    {
        d = date.substr(0, 2); m = date.substr(3, 2); y = date.substr(6, 4);
    }
    else
    {
        return false;
    }

    if (!allDigits(d) || !allDigits(m) || !allDigits(y)) return false;

    year = stoi(y);
    month = stoi(m);
    day = stoi(d);
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

string dateKey(const string& date)
{
    int d, m, y;
    if (!parseDate(date, d, m, y)) return "";

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d", y, m, d);
    return buffer;
}

string addDays(const string& date, int n)
{
    int day, month, year;
    if (!parseDate(date, day, month, year)) return "";

    tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day + n;
    t.tm_hour = 12;
    t.tm_isdst = -1;
    if (mktime(&t) == (time_t)-1) return "";

    return makeDate(t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
}

bool daysBetween(const string& from, const string& to, int& days)
{
    int d1, m1, y1, d2, m2, y2;
    if (!parseDate(from, d1, m1, y1) || !parseDate(to, d2, m2, y2)) return false;

    tm a = {};
    a.tm_year = y1 - 1900; a.tm_mon = m1 - 1; a.tm_mday = d1; a.tm_hour = 12; a.tm_isdst = -1;
    tm b = {};
    b.tm_year = y2 - 1900; b.tm_mon = m2 - 1; b.tm_mday = d2; b.tm_hour = 12; b.tm_isdst = -1;

    time_t x = mktime(&a);
    time_t y = mktime(&b);
    if (x == (time_t)-1 || y == (time_t)-1) return false;

    days = (int)lround(difftime(y, x) / 86400.0);
    return true;
}