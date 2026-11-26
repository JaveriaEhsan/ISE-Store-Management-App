#ifndef CATEGORYREPOSITORY_H
#define CATEGORYREPOSITORY_H
#include <string>
#include <vector>
#include "data/ICategoryRepository.h"

// Reads and writes PCategory.txt. Line format: categoryId name acronym#
class CategoryRepository : public ICategoryRepository
{
private:
    std::string filePath;   // path of PCategory.txt
    // Checks id (2 digits), name and acronym. Fills error if invalid.
    bool validate(const Category& category, std::string& error) const;
    // Overwrites the whole file with the given categories. Returns false if it cannot be written.
    bool saveAll(const std::vector<Category>& all);
public:
    explicit CategoryRepository(const std::string& filePath);

    std::vector<Category> getAll() override;
    bool findById(const std::string& id, Category& out) override;
    bool add(const Category& category, std::string& error) override;
    bool update(const Category& category, std::string& error) override;
    std::string nextCategoryId() override;
};

#endif
