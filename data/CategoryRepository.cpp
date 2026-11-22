#include "data/CategoryRepository.h"
#include <fstream>
#include <sstream>
using namespace std;
CategoryRepository::CategoryRepository(const string& path)
    : filePath(path)
{
}
vector<Category> CategoryRepository::getAll()
{
    vector<Category> result;
    ifstream file(filePath);
    string line;
    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty() && line.back() == '#') {
            line.pop_back();
        }

        istringstream in(line);
        Category c;
        if (in >> c.categoryId >> c.name >> c.acronym) {
            result.push_back(c);
        }
        // malformed lines are skipped
    }
    return result;
}

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

// Placeholders: replaced with real code in the next commit
bool CategoryRepository::add(const Category&, string& error)
{
    error = "Not implemented yet.";
    return false;
}

bool CategoryRepository::update(const Category&, string& error)
{
    error = "Not implemented yet.";
    return false;
}