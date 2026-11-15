#ifndef TEXTFORMAT_H   // include guard
#define TEXTFORMAT_H

#include <string>
using namespace std;

string fixText(string text);   // empty -> "-", space -> "_"
string toText(double value);   // 450.5 -> "450.50"

#endif