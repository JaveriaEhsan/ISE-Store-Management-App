#include "ProductRepository.h"
#include "TextFormat.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

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

// Lower case copy of the text, used for case-insensitive comparison
static string lower(string text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] >= 'A' && text[i] <= 'Z')
        {
            text[i] = (char)(text[i] - 'A' + 'a');
        }
    }
    return text;
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

// Line format:
// id categoryId barcode acronym name unit price tax discount quantity path#
bool ProductRepository::parseLine(string line, Product& p, string& error) const
{
    size_t hash = line.find_last_of('#');
    if (hash == string::npos)
    {
        error = "missing # at the end of the line";
        return false;
    }
    line.erase(hash + 1);   // drop anything after the last #

    // splitLine removes the # at the end and splits the line into words
    vector<string> words = splitLine(line);

    // 4 words at the start + at least 1 word of name + 6 words at the end
    if (words.size() < 11)
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
    string categoryId = words[1];
    string barcode = words[2];
    string acronym = words[3];
    string unit = words[n - 6];
    string path = words[n - 1];

    if (categoryId.size() != 2 || !isDigits(categoryId) || categoryId[0] == '0')
    {
        error = "category id must be 2 digits and must not start with 0";
        return false;
    }
    if (barcode.size() != 6 || !isDigits(barcode) || barcode[0] == '0')
    {
        error = "barcode must be 6 digits and must not start with 0";
        return false;
    }
    if (barcode.substr(0, 2) != categoryId)
    {
        error = "barcode must start with the category id";
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

    string name = buildName(words, 4, n - 6);
    if (!hasLetterOrDigit(name))
    {
        error = "name is empty";
        return false;
    }

    p = Product(id, barcode, acronym, name, unit, values[0], values[1], values[2], values[3], path, categoryId);
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

    for (size_t i = 0; i < products.size(); i++)
    {
        out << toLine(products[i]) << endl;
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

bool ProductRepository::findById(const string& id, Product& result) const
{
    for (size_t i = 0; i < products.size(); i++)
    {
        if (products[i].getId() == id)
        {
            result = products[i];
            return true;
        }
    }
    return false;   // not found
}

// Not case sensitive. Acronyms are not forced to be unique, so the first match is returned.
bool ProductRepository::findByAcronym(const string& acronym, Product& result) const
{
    string wanted = lower(acronym);
    for (size_t i = 0; i < products.size(); i++)
    {
        if (lower(products[i].getAcronym()) == wanted)
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

// Replaces the product with the same barcode. The new values are checked with the
// same rules as add() and load(), and the id stored in the file is kept.
// If the file cannot be written, the old product is put back.
bool ProductRepository::update(const Product& product, string& error)
{
    error = "";

    for (size_t i = 0; i < products.size(); i++)
    {
        if (products[i].getBarcode() != product.getBarcode())
        {
            continue;
        }

        Product withId(products[i].getId(), product.getBarcode(), product.getAcronym(),
            product.getName(), product.getUnit(), product.getPrice(), product.getTax(),
            product.getDiscount(), product.getQuantity(), product.getBarcodePath(),
            product.getCategoryId());

        Product checked;
        if (!parseLine(toLine(withId), checked, error))
        {
            return false;   // invalid values never reach the file
        }

        Product old = products[i];
        products[i] = checked;
        if (!save())
        {
            products[i] = old;
            error = "Could not save " + filename + ".";
            return false;
        }
        return true;
    }

    error = "Product not found.";
    return false;
}

// One product as a line of Product.txt (same text helpers as the Order and Bill files).
// price, tax and discount use toText (2 decimals); the quantity has 0 decimals for
// count products and 3 for weight products, so a weight like 1.250 is not rounded.
string ProductRepository::toLine(const Product& p) const
{
    ostringstream quantity;
    quantity << fixed << setprecision(p.getUnit() == "count" ? 0 : 3) << p.getQuantity();

    ostringstream out;
    out << p.getId() << " " << p.getCategoryId() << " " << p.getBarcode() << " " << p.getAcronym() << " "
        << fixText(p.getName()) << " " << p.getUnit() << " " << toText(p.getPrice()) << " "
        << toText(p.getTax()) << " " << toText(p.getDiscount()) << " " << quantity.str() << " "
        << p.getBarcodePath() << "#";
    return out.str();
}

// The next free id: the biggest id in the file plus 1
string ProductRepository::nextId() const
{
    long long biggest = 0;
    for (size_t i = 0; i < products.size(); i++)
    {
        try
        {
            long long id = stoll(products[i].getId());
            if (id > biggest)
            {
                biggest = id;
            }
        }
        catch (...) {}   // an id that is too long is ignored
    }
    return to_string(biggest + 1);
}

// Note: this only checks the format of the category id. The category must be
// verified by the caller (AddProductDialog picks it from the category list).
bool ProductRepository::add(const Product& product, string& error)
{
    error = "";

    Product withId(nextId(), product.getBarcode(), product.getAcronym(), product.getName(),
        product.getUnit(), product.getPrice(), product.getTax(),
        product.getDiscount(), product.getQuantity(), product.getBarcodePath(),
        product.getCategoryId());

    // check it with the same rules that are used when the file is loaded
    Product checked;
    if (!parseLine(toLine(withId), checked, error))
    {
        return false;
    }
    if (isDuplicate(checked))
    {
        error = "A product with this barcode already exists.";
        return false;
    }

    products.push_back(checked);
    if (!save())
    {
        products.pop_back();
        error = "Could not save " + filename + ".";
        return false;
    }
    return true;
}

// Barcode = category id (2 digits) + serial (4 digits, 0001 to 9999).
// Takes the biggest serial used in this category and adds 1.
// Returns "" if the category id is invalid or all 9999 serials are used.
string ProductRepository::nextBarcode(const string& categoryId) const
{
    // same rule as the category: exactly 2 digits, not starting with 0
    if (categoryId.size() != 2 || !isDigits(categoryId) || categoryId[0] == '0')
    {
        return "";
    }

    int biggestSerial = 0;
    for (size_t i = 0; i < products.size(); i++)
    {
        string barcode = products[i].getBarcode();
        if (barcode.size() != 6 || barcode.substr(0, 2) != categoryId)
        {
            continue;   // a barcode of another category
        }
        int serial = stoi(barcode.substr(2));   // the barcode is 6 digits, so this is safe
        if (serial > biggestSerial)
        {
            biggestSerial = serial;
        }
    }

    int next = biggestSerial + 1;
    if (next > 9999)
    {
        return "";   // this category is full
    }

    // pad the serial to 4 digits, for example 5 becomes 0005
    string serial = to_string(next);
    while (serial.size() < 4)
    {
        serial = "0" + serial;
    }
    return categoryId + serial;
}