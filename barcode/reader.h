#ifndef READER_H
#define READER_H

#include <string>
using namespace std;

// Owner: Member 3
// Loads the image and returns one middle row as a string of '1' (black) and '0' (white).
// Example: "0000111000111..."
// Returns an empty string if the file is missing or not an image.
string readBarcodeRow(const string& filename);

#endif