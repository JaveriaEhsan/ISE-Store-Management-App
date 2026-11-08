#ifndef SHARED_H
#define SHARED_H
using namespace std;
#include <string>

// ===== Shared settings for the whole project =====

// Code table: digits (0-9) -> 7-bit pattern (1 = black, 0 = white)
const string Code_Table[10] = {
    "0001101", "0011001", "0010011", "0111101", "0100011",
    "0110001", "0101111", "0111011", "0110111", "0001011"
};

// Start and end marker
const string Marker = "101";

// Image settings
const int Bar_Width    = 3;    // pixels per bit
const int Image_Height = 80;   // image height in pixels
const int Quiet_Zone   = 5;    // white margin, in bits, on each side

// Barcode size (used by the decoder)
const int Digit_Count    = 6;  // digits in the number
const int Bits_Per_Digit = 7;  // bits for one digit
const int Total_Bits     = 48; // 3 + (6 x 7) + 3

#endif