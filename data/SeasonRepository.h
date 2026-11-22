#ifndef SEASONREPOSITORY_H
#define SEASONREPOSITORY_H

#include "ISeasonRepository.h"

// Reads and writes Seasons.txt
class SeasonRepository : public ISeasonRepository
{
private:
    vector<Season> seasons;     // the seasons we read from the file

    bool saveAll() const;       // writes the whole file again

public:
    void load() override;
    vector<Season> getAll() const override;
    bool add(const Season& season, string& error) override;
    bool setActive(int index, bool active) override;
};

#endif