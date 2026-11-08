#ifndef DECODER_H
#define DECODER_H

#include <string>
using namespace std;

// Owner: Member 4
// Takes the row string from readBarcodeRow.
// On success: returns true and puts the 6-digit number in "number".
// On failure: returns false and puts a message in "error".
bool decodeRow(const string& row, string& number, string& error);

#endif