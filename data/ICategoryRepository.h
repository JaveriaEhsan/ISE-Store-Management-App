#ifndef ICATEGORYREPOSITORY_H
#define ICATEGORYREPOSITORY_H
#include <string>
#include <vector>
#include "models/category.h"
using namespace std;
class ICategoryRepository
{
public:
    virtual ~ICategoryRepository() = default;
    // All valid categories from PCategory.txt
    virtual std::vector<Category> getAll() = 0;
    // Returns true and fills out if the id exists
    virtual bool findById(const string& id, Category& out) = 0;
    // Returns false and fills error if the category is invalid or the id already exists
    virtual bool add(const Category& category, string& error) = 0;
    // Changes name and acronym of an existing id
    virtual bool update(const Category& category,string& error) = 0;
};

#endif