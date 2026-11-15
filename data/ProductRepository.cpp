#include "ProductRepository.h"
#include <fstream>
#include <sstream>

// Converts text to a number. Returns false if the text is not a clean number.
static bool toNumber(const string& text, double& value)
{
    stringstream ss(text);
    ss >> value;
    return !ss.fail() && ss.eof();
}

// True if the text is not empty and has only digits
static bool isDigits(const string& text)
{
    if (text.empty())
    {
        return false;
    }
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }
    }
    return true;
}

// True if the text has at least one letter
static bool hasLetter(const string& text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        if ((text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z'))
        {
            return true;
        }
    }
    return false;
}

// True if the text has only letters and digits
static bool isLettersDigits(const string& text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        bool letter = (text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z');
        bool digit = text[i] >= '0' && text[i] <= '9';
        if (!letter && !digit)
        {
            return false;
        }
    }
    return true;
}

// True if the text has at least one letter or digit
static bool hasLetterOrDigit(const string& text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        bool letter = (text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z');
        bool digit = text[i] >= '0' && text[i] <= '9';
        if (letter || digit)
        {
            return true;
        }
    }
    return false;
}

// True if the text ends with the given ending and has something before it
static bool endsWith(const string& text, const string& ending)
{
    if (text.size() <= ending.size())
    {
        return false;
    }
    return text.compare(text.size() - ending.size(), ending.size(), ending) == 0;
}

// Splits a line into words
static vector<string> splitWords(const string& line)
{
    vector<string> words;
    stringstream ss(line);
    string word;
    while (ss >> word)
    {
        words.push_back(word);
    }
    return words;
}

// Joins words[from] to words[to - 1] with spaces; _ also becomes a space
static string buildName(const vector<string>& words, size_t from, size_t to)
{
    string name = "";
    for (size_t i = from; i < to; i++)
    {
        if (i > from)
        {
            name = name + " ";
        }
        name = name + words[i];
    }
    for (size_t i = 0; i < name.size(); i++)
    {
        if (name[i] == '_')
        {
            name[i] = ' ';
        }
    }
    return name;
}

ProductRepository::ProductRepository(const string& filename)
{
    this->filename = filename;
}

bool ProductRepository::parseLine(string line, Product& p, string& error) const
{
    size_t hash = line.find_last_of('#');
    if (hash == string::npos)
    {
        error = "missing # at the end of the line";
        return false;
    }
    line.erase(hash);   // remove the # at the end

    vector<string> words = splitWords(line);

    // 3 words at the start + at least 1 word of name + 6 words at the end
    if (words.size() < 10)
    {
        error = "too few fields";
        return false;
    }

    size_t n = words.size();

    // the last 6 words must not look like a path, otherwise the path has a space in it
    for (size_t i = n - 6; i <= n - 2; i++)
    {
        if (words[i].find('/') != string::npos || words[i].find('\\') != string::npos)
        {
            error = "path must not contain spaces";
            return false;
        }
    }

    string id = words[0];
    string barcode = words[1];
    string acronym = words[2];
    string unit = words[n - 6];
    string path = words[n - 1];

    if (barcode.size() != 6 || !isDigits(barcode) || barcode[0] == '0')
    {
        error = "barcode must be 6 digits and must not start with 0";
        return false;
    }
    if (!isDigits(id))
    {
        error = "id must be a number";
        return false;
    }
    if (!hasLetter(acronym) || !isLettersDigits(acronym) || acronym.size() > 8)
    {
        error = "acronym must have a letter, use only letters and digits, and be at most 8 characters";
        return false;
    }
    if (unit != "count" && unit != "weight")
    {
        error = "unit must be count or weight";
        return false;
    }

    const char* names[4] = { "price", "tax", "discount", "quantity" };
    double values[4];
    for (int i = 0; i < 4; i++)
    {
        if (!toNumber(words[n - 5 + i], values[i]))
        {
            error = string(names[i]) + " is not a number";
            return false;
        }
        if (values[i] < 0)
        {
            error = string(names[i]) + " cannot be negative";
            return false;
        }
    }

    if (values[0] <= 0)
    {
        error = "price must be greater than 0";
        return false;
    }
    if (values[2] > values[0])
    {
        error = "discount cannot be more than the price";
        return false;
    }
    if (unit == "count" && values[3] != (long long)values[3])
    {
        error = "quantity of a count product must be a whole number";
        return false;
    }
    if (!endsWith(path, ".png"))
    {
        error = "path must end with .png";
        return false;
    }

    string name = buildName(words, 3, n - 6);
    if (!hasLetterOrDigit(name))
    {
        error = "name is empty";
        return false;
    }

    p = Product(id, barcode, acronym, name, unit, values[0], values[1], values[2], values[3], path);
    if (!p.isValid())
    {
        error = "values are not valid";
        return false;
    }
    return true;
}

bool ProductRepository::isDuplicate(const Product& p) const
{
    for (size_t i = 0; i < products.size(); i++)
    {
        if (products[i].getBarcode() == p.getBarcode() || products[i].getId() == p.getId())
        {
            return true;
        }
    }
    return false;
}

bool ProductRepository::load()
{
    ifstream in(filename);
    if (!in.is_open())
    {
        return false;
    }

    products.clear();
    skippedLines.clear();
    errors.clear();

    string line;
    int lineNumber = 0;
    while (getline(in, line))
    {
        lineNumber++;

        // Notepad can add a hidden marker at the start of the file, remove it
        if (lineNumber == 1 && line.size() >= 3 && line.substr(0, 3) == "\xEF\xBB\xBF")
        {
            line = line.substr(3);
        }

        // ignore empty lines
        if (line.find_first_not_of(" \t\r") == string::npos)
        {
            continue;
        }

        Product p;
        string error;
        bool ok = parseLine(line, p, error);
        if (ok && isDuplicate(p))
        {
            ok = false;
            error = "barcode or id is already used by an earlier line";
        }

        if (!ok)
        {
            skippedLines.push_back(line);
            errors.push_back("Line " + to_string(lineNumber) + ": " + error);
            continue;
        }
        products.push_back(p);
    }
    return true;
}

int ProductRepository::getSkippedCount() const
{
    return (int)skippedLines.size();
}

vector<string> ProductRepository::getErrors() const
{
    return errors;
}

bool ProductRepository::save() const
{
    ofstream out(filename);
    if (!out.is_open())
    {
        return false;
    }

    out.precision(10);   // avoids numbers like 1e+06
    for (size_t i = 0; i < products.size(); i++)
    {
        const Product& p = products[i];

        // write the name with _ instead of spaces, same as the file format
        string name = p.getName();
        for (size_t j = 0; j < name.size(); j++)
        {
            if (name[j] == ' ')
            {
                name[j] = '_';
            }
        }

        out << p.getId() << " " << p.getBarcode() << " " << p.getAcronym() << " "
            << name << " " << p.getUnit() << " " << p.getPrice() << " "
            << p.getTax() << " " << p.getDiscount() << " " << p.getQuantity() << " "
            << p.getBarcodePath() << "#" << endl;
    }

    // keep the lines we could not read, so they are not lost
    for (size_t i = 0; i < skippedLines.size(); i++)
    {
        out << skippedLines[i] << endl;
    }
    return true;
}

bool ProductRepository::findByBarcode(const string& barcode, Product& result) const
{
    for (size_t i = 0; i < products.size(); i++)
    {
        if (products[i].getBarcode() == barcode)
        {
            result = products[i];
            return true;
        }
    }
    return false;   // not found
}

vector<Product> ProductRepository::getAll() const
{
    return products;
}

bool ProductRepository::update(const Product& product)
{
    for (size_t i = 0; i < products.size(); i++)
    {
        if (products[i].getBarcode() == product.getBarcode())
        {
            products[i] = product;
            return save();   // write the change to the file
        }
    }
    return false;   // product not found
}