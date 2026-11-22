#ifndef CATEGORYREPOSITORY_H
#define CATEGORYREPOSITORY_H
#include <string>
#include <vector>
#include "data/ICategoryRepository.h"
using namespace std;

// Reads and writes PCategory.txt. Line format: categoryId name acronym#
class CategoryRepository : public ICategoryRepository
{
private:
    string filePath;   // path of PCategory.txt
    // Checks id (2 digits), name and acronym. Fills error if invalid.
    bool validate(const Category& category, string& error) const;
    // Overwrites the whole file with the given categories
    void saveAll(const vector<Category>& all);
public:
    explicit CategoryRepository(const string& filePath);

    vector<Category> getAll() override;
    bool findById(const string& id, Category& out) override;
    bool add(const Category& category, string& error) override;
    bool update(const Category& category, string& error) override;
};

#endif
