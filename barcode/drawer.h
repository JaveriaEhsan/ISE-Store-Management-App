#ifndef DRAWER_H
#define DRAWER_H

#include <string>
using namespace std;

// Owner: Member 2
// Prints the barcode on the console
void printConsole(const string& bits);

// Draws the barcode, shows it on screen and saves it as filename. Returns true if saved.
bool createBarcodeImage(const string& bits, const string& filename);

#endif