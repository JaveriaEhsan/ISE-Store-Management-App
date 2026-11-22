#ifndef TEXTFORMAT_H   // include guard
#define TEXTFORMAT_H

#include <string>

std::string fixText(std::string text);   // empty -> "-", space -> "_"
std::string toText(double value);   // 450.5 -> "450.50"

#endif