#include "TextFormat.h"

#include <iomanip>   
#include <sstream>

using namespace std;

// empty = "-", space = "_"
string fixText(string text)
{
    if (text.empty())
    {
        return "-";
    }

    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == ' ')
        {
            text[i] = '_';
        }
    }
    return text;
}

// Turns a number into text with 2 decimals, like 450.5 -> "450.50"
string toText(double value)
{
    ostringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

// Removes the last '#' and splits the line into words
vector<string> splitLine(string line)
{
    if (!line.empty() && line.back() == '#')
    {
        line.pop_back();
    }

    istringstream in(line);
    vector<string> fields;
    string word;

    while (in >> word)
    {
        fields.push_back(word);
    }

    return fields;
}