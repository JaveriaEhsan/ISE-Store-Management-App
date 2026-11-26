#include "data/CategoryRepository.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

CategoryRepository::CategoryRepository(const string& path)
    : filePath(path)
{
}

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

// Case-insensitive comparison, so "Dairy" and "DAIRY" count as the same
static bool sameText(const string& a, const string& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++)
    {
        if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }
    return true;
}

// Checks that no OTHER category already uses this name or acronym
static bool nameOrAcronymTaken(const vector<Category>& all, const Category& c, string& error)
{
    for (const Category& existing : all)
    {
        if (existing.categoryId == c.categoryId)
        {
            continue;   // the category itself (needed for update)
        }
        if (sameText(existing.name, c.name))
        {
            error = "Another category already has this name.";
            return true;
        }
        if (sameText(existing.acronym, c.acronym))
        {
            error = "Another category already has this acronym.";
            return true;
        }
    }
    return false;
}

// Rules: id is 2 digits and does not start with 0, name and acronym are not empty and have no '#'
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
    // The id is the start of the barcode, and barcodes cannot start with 0
    if (category.categoryId[0] == '0')
    {
        error = "Category id cannot start with 0.";
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
    // '#' ends a line in the file, so it cannot be part of a name
    if (category.name.find('#') != string::npos ||
        category.acronym.find('#') != string::npos)
    {
        error = "Name and acronym cannot contain #.";
        return false;
    }
    return true;
}

// Rewrites PCategory.txt from scratch, one line per category.
// Returns false if the file could not be written.
bool CategoryRepository::saveAll(const vector<Category>& all)
{
    ofstream file(filePath, ios::trunc);
    for (const Category& c : all)
    {
        file << c.categoryId << " " << c.name << " " << c.acronym << "#\n";
    }
    return file.good();
}

// Adds a new category. Fails if invalid, the id/name/acronym already exists,
// or the file cannot be written.
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
    if (nameOrAcronymTaken(all, c, error))
    {
        return false;
    }

    all.push_back(c);
    if (!saveAll(all))
    {
        error = "Could not write to the category file.";
        return false;
    }
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
    if (nameOrAcronymTaken(all, c, error))
    {
        return false;
    }

    for (Category& existing : all)
    {
        if (existing.categoryId == c.categoryId)
        {
            existing = c;
            if (!saveAll(all))
            {
                error = "Could not write to the category file.";
                return false;
            }
            return true;
        }
    }

    error = "Category not found.";
    return false;
}

// The next free id: the biggest id in the file plus 1 (the first one is 10).
// Returns "" if the ids are used up to 99.
string CategoryRepository::nextCategoryId()
{
    int biggest = 9;
    for (const Category& c : getAll())
    {
        bool twoDigits = c.categoryId.size() == 2 &&
            isdigit(static_cast<unsigned char>(c.categoryId[0])) &&
            isdigit(static_cast<unsigned char>(c.categoryId[1]));
        if (!twoDigits)
        {
            continue;   // an id that is not 2 digits is ignored
        }
        int id = stoi(c.categoryId);
        if (id > biggest)
        {
            biggest = id;
        }
    }
    int next = (biggest / 10 + 1) * 10;
    if (next > 99) {
        return "";
    }
   
    return to_string(next);
}
