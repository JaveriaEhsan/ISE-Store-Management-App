#ifndef ENCODER_H
#define ENCODER_H

#include <string>
using namespace std;

// Owner: Member 1
// Returns true if the input is exactly 6 digits and the first digit is not 0
bool isvalidInput(const string& digits);

// Takes a valid 6-digit number, returns the 48-bit string (start + 6 digits + end)
string Encode(const string& number);

#endif