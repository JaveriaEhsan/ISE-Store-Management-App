#ifndef ISEASONREPOSITORY_H
#define ISEASONREPOSITORY_H

#include <string>
#include <vector>
#include "../models/season.h"

class ISeasonRepository
{
public:
    virtual ~ISeasonRepository() {}

    // read Seasons.txt again
    virtual void load() = 0;

    // all seasons, in file order
    virtual std::vector<Season> getAll() const = 0;

    // add a new season; false and a message in error if the dates or percent are bad
    virtual bool add(const Season& season, std::string& error) = 0;

    // switch a season on or off
    virtual bool setActive(int index, bool active) = 0;

    // Find the season that applies on this date (YYYY-MM-DD).
    // Only active seasons count. If more than one fits, the highest percent wins.
    // Returns false if no season fits.
    virtual bool findActiveSeason(const std::string& date, Season& found) = 0;
};

#endif 