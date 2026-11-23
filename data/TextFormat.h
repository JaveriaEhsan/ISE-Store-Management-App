#ifndef TEXTFORMAT_H
#define TEXTFORMAT_H

#include <string>
#include <vector>

std::string fixText(std::string text);   // empty = "-", space = "_"
std::string toText(double value);        // number with 2 decimals, like 450.5 -> "450.50"

// Removes the last '#' and splits a file line into words
std::vector<std::string> splitLine(std::string line);

#endif