#include "SeasonRepository.h"
#include "TextFormat.h"      
#include "../config.h"       

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// True if the text looks like YYYY-MM-DD with a real month and day
static bool validDate(const string& d)
{
    if (d.size() != 10 || d[4] != '-' || d[7] != '-')
    {
        return false;
    }

    for (int i = 0; i < d.size(); i++)
    {
        if (i != 4 && i != 7 && !isdigit((unsigned char)d[i]))
        {
            return false;
        }
    }

    int month = stoi(d.substr(5, 2));
    int day = stoi(d.substr(8, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

// Read Seasons.txt: remove the #, split by spaces, skip bad lines
void SeasonRepository::load()
{
    seasons.clear();

    ifstream file(SEASON_FILE);
    string line;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        if (line.back() == '#')
        {
            line.pop_back();
        }

        istringstream in(line);
        Season s;
        string percent, active;

        if (!(in >> s.startDate >> s.endDate >> percent >> active))
        {
            continue;               // not enough fields
        }

        try
        {
            s.discountPercent = stod(percent);
        }
        catch (...)
        {
            continue;               // percent is not a number
        }

        if (!validDate(s.startDate) || !validDate(s.endDate))
        {
            continue;
        }

        if (active != "true" && active != "false")
        {
            continue;
        }
        s.isActive = (active == "true");

        seasons.push_back(s);
    }
}

vector<Season> SeasonRepository::getAll() const
{
    return seasons;
}

// Check the season, then add one line at the end of the file
bool SeasonRepository::add(const Season& season, string& error)
{
    load();  

    if (!validDate(season.startDate) || !validDate(season.endDate))
    {
        error = "Dates must look like 2026-11-21.";
        return false;
    }

    if (season.startDate > season.endDate)
    {
        error = "The start date cannot be after the end date.";
        return false;
    }

    if (season.discountPercent <= 0 || season.discountPercent > 100)
    {
        error = "The discount must be more than 0 and at most 100.";
        return false;
    }

    ofstream file(SEASON_FILE, ios::app);
    if (!file.is_open())
    {
        error = "Could not open the seasons file.";
        return false;
    }

    file << season.startDate << " "
         << season.endDate << " "
         << toText(season.discountPercent) << " "
         << (season.isActive ? "true" : "false")
         << "#" << "\n";

    if (!file.good())
    {
        error = "Could not write to the seasons file.";
        return false;
    }

    seasons.push_back(season);
    return true;
}

// Change one season, then write the whole file again
bool SeasonRepository::setActive(int index, bool active)
{
    load();   

    if (index < 0 || index >= (int)seasons.size())
    {
        return false;
    }

    seasons[index].isActive = active;
    return saveAll();
}

bool SeasonRepository::saveAll() const
{
    ofstream file(SEASON_FILE);     // no ios::app, so the file starts fresh
    if (!file.is_open())
    {
        return false;
    }

    for (const Season& s : seasons)
    {
        file << s.startDate << " "
             << s.endDate << " "
             << toText(s.discountPercent) << " "
             << (s.isActive ? "true" : "false")
             << "#" << "\n";
    }

    return file.good();
}
// Find the season that applies on this date (YYYY-MM-DD)
bool SeasonRepository::findActiveSeason(const string& date, Season& found)
{
    if (!validDate(date))
    {
        return false;
    }

    load();   // use what is really in the file

    bool any = false;

    for (const Season& s : seasons)
    {
        if (!s.isActive)
        {
            continue;                       // switched off
        }

        if (date < s.startDate || date > s.endDate)
        {
            continue;                       // the date is outside this season
        }

        if (!any || s.discountPercent > found.discountPercent)
        {
            found = s;                      // keep the highest percent
            any = true;
        }
    }

    return any;
}