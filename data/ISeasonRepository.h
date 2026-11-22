#ifndef ISEASONREPOSITORY_H
#define ISEASONREPOSITORY_H

#include <string>
#include <vector>
#include "../models/season.h"
using namespace std;

class ISeasonRepository
{
public:
    virtual ~ISeasonRepository() {}

    // read Seasons.txt again
    virtual void load() = 0;

    // all seasons, in file order
    virtual vector<Season> getAll() const = 0;

    // add a new season; false and a message in error if the dates or percent are bad
    virtual bool add(const Season& season, string& error) = 0;

    // switch a season on or off
    virtual bool setActive(int index, bool active) = 0;
};

#endif 