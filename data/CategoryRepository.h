#ifndef CATEGORYREPOSITORY_H
#define CATEGORYREPOSITORY_H
#include <string>
#include <vector>
#include "data/ICategoryRepository.h"
using namespace std;
class CategoryRepository : public ICategoryRepository
{
private:
    string filePath;
public:
    explicit CategoryRepository(const string& filePath);
   vector<Category> getAll() override;
    bool findById(const string& id, Category& out) override;
    bool add(const Category& category, string& error) override;
    bool update(const Category& category, string& error) override;

};

#endif
