#include "DateUtil.h"
#include <cmath>
#include <ctime>

using namespace std;

string todayDate()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", t);
    return buffer;
}

string nowTime()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M", t);
    return buffer;
}

string dateKey(const string& date)
{
    if (date.size() != 10 || date[2] != '-' || date[5] != '-') return "";

    string key = date.substr(6, 4) + date.substr(3, 2) + date.substr(0, 2);
    for (size_t i = 0; i < key.size(); i++)
    {
        if (key[i] < '0' || key[i] > '9') return "";
    }
    return key;
}

bool parseDate(const string& date, int& day, int& month, int& year)
{
    string key = dateKey(date);
    if (key.empty()) return false;

    year = stoi(key.substr(0, 4));
    month = stoi(key.substr(4, 2));
    day = stoi(key.substr(6, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
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

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", &t);
    return buffer;
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