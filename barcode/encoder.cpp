#include "encoder.h"
#include "../shared.h"

bool isvalidInput(const string& digits)
{
    if(digits.length() != 6)
    {
        return false;
    }

    if(digits[0] == '0')
    {
        return false;
    }

    for(int i = 0; i < digits.length(); i++)
    {
        if(digits[i] < '0' || digits[i] > '9')
        {
            return false;
        }
    }

    return true;
}

string Encode(const string& number)
{
    if(!isvalidInput(number))
    {
        return "";
    }

    string bits = Marker;

    for(int i = 0; i < 6; i++)
    {
        int digit = number[i] - '0';

        bits = bits + Code_Table[digit];
    }

    bits = bits + Marker;

    return bits;
}