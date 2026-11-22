#include "data/CategoryRepository.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
CategoryRepository::CategoryRepository(const string& path) : filePath(path){}

// Reads every line of PCategory.txt and returns the valid ones
vector<Category> CategoryRepository::getAll()
{
    vector<Category> result;
    ifstream file(filePath);
    string line;

    while (getline(file, line))
    {
        // Remove the Windows line ending, then the closing '#'
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty() && line.back() == '#') {
            line.pop_back();
        }

        // Split by spaces into the three fields
        istringstream in(line);
        Category c;
        if (in >> c.categoryId >> c.name >> c.acronym) {
            result.push_back(c);
        }
        // Lines with missing fields are skipped
    }
    return result;
}

// Searches by id. Returns true and fills out when found.
bool CategoryRepository::findById(const string& id, Category& out)
{
    for (const Category& c : getAll())
    {
        if (c.categoryId == id)
        {
            out = c;
            return true;
        }
    }
    return false;
}

// Names are stored with _ instead of spaces, so the file stays space separated.
// Without this, "Dairy Products" would be read back as two fields.
static void spacesToUnderscores(Category& c)
{
    replace(c.name.begin(), c.name.end(), ' ', '_');
    replace(c.acronym.begin(), c.acronym.end(), ' ', '_');
}

// Rules: id is exactly 2 digits, name and acronym are not empty
bool CategoryRepository::validate(const Category& category, string& error) const
{
    bool twoDigits = category.categoryId.size() == 2 &&
        all_of(category.categoryId.begin(), category.categoryId.end(),
            [](unsigned char ch) { return isdigit(ch) != 0; });

    if (!twoDigits)
    {
        error = "Category id must be exactly 2 digits.";
        return false;
    }
    if (category.name.empty())
    {
        error = "Name is required.";
        return false;
    }
    if (category.acronym.empty())
    {
        error = "Acronym is required.";
        return false;
    }
    return true;
}

// Rewrites PCategory.txt from scratch, one line per category
void CategoryRepository::saveAll(const vector<Category>& all)
{
    ofstream file(filePath, ios::trunc);
    for (const Category& c : all)
    {
        file << c.categoryId << " " << c.name << " " << c.acronym << "#\n";
    }
}

// Adds a new category. Fails if invalid or the id already exists.
bool CategoryRepository::add(const Category& category, string& error)
{
    Category c = category;
    spacesToUnderscores(c);
    if (!validate(c, error))
    {
        return false;
    }

    vector<Category> all = getAll();
    for (const Category& existing : all)
    {
        if (existing.categoryId == c.categoryId)
        {
            error = "This category id already exists.";
            return false;
        }
    }

    all.push_back(c);
    saveAll(all);
    return true;
}

// Changes name and acronym of an existing category (the id never changes)
bool CategoryRepository::update(const Category& category, string& error)
{
    Category c = category;
    spacesToUnderscores(c);
    if (!validate(c, error))
    {
        return false;
    }

    vector<Category> all = getAll();
    for (Category& existing : all)
    {
        if (existing.categoryId == c.categoryId)
        {
            existing = c;
            saveAll(all);
            return true;
        }
    }

    error = "Category not found.";
    return false;
}