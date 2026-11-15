#include "TextFormat.h"   

#include <iomanip>   // set precision
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
        if (text[i] == ' ')                    // space found
        {
            text[i] = '_';                     // replace with underscore
        }
    }
    return text;
}
// Turns a number into text with 2 decimals, like 450.5 -> "450.50"
string toText(double value)
{
    ostringstream out;
    out << fixed << setprecision(2) << value;  // 2 decimals
    return out.str();
}