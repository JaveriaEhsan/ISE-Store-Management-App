#include "IdGenerator.h"   
#include "../config.h"  
#include <ctime>     
#include <fstream>   
#include <string>

using namespace std;

// Reads the file and returns its last non-empty line ("" if no file)
static string lastLine(const string& path)
{
    ifstream in(path);
    string line, last;
    while (getline(in, line))
    {
        if (!line.empty()) 
        last = line;
    }
    return last;
}

// Adds zeros in front of the text until it is as long as width: "3" -> "000003"
static string pad(string text, int width)
{
    while ((int)text.size() < width)
    {
        text = "0" + text;
    }
    return text;
}

// Takes the first digits of the last line as a number (0 if file is empty or bad)
static int lastSerial(const string& path, int digits)
{
    string line = lastLine(path);
    if ((int)line.size() < digits)
    {
        return 0;
    }

    try
    {
        int value = stoi(line.substr(0, digits));
        if (value < 0)
        {
            return 0;       
        }
        return value;
    }
    catch (...)             
    {
        return 0;
    }
}

// Makes the next order id: last 6-digit serial + 1, then today's day (8 digits)
string IdGenerator::nextOrderId()
{
    int serial = lastSerial(ORDER_FILE, 6) + 1;   // last serial + 1

    auto now = time(nullptr);                            // current time
    tm* local = localtime(&now);                         // split into day, month...

    return pad(to_string(serial), 6) + pad(to_string(local->tm_mday), 2);
}

// Makes the next bill id: last 9-digit bill id + 1
string IdGenerator::nextBillId()
{
    int serial = lastSerial(BILL_FILE, 9)+ 1;    // last bill id + 1
    return pad(to_string(serial), 9);
}



