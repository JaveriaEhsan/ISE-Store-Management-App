#include "decoder.h"
#include "../shared.h"
#include <iostream>
using namespace std;

bool decodeRow(const string& row, string& number, string& error)
{
    number = "";
    error = "";
    // STEP 1: Find the first black pixel
    int start = 0;
    while (start < row.size() && row[start] == '0') {
        start++;
    }
    if (start == row.size()){
        error = "No barcode found.";
        return false;
    }
    // STEP 2: Measure the first black bar
    int width = 0;
    while (start+ width<row.size()&&row[start + width] == '1') {
        width++;
    }
    if (width ==0){
        error = "Invalid bar width." ;
        return false;
    }
    // STEP 3: Convert pixels back into 48 bits
    string bits = "";
    for (int i = 0; i < 48; i++) {
        int position = start + width / 2 + i * width;
        if (position >= row.size()){
            error = "Incomplete barcode.";
            return false;
        }
        bits += row[position];
    }
    // STEP 4: Check start marker
    if (bits.substr(0, 3) != Marker){
        error = "Invalid start marker.";
        return false;
    }
    // STEP 5: Check end marker
    if (bits.substr(45, 3) != Marker){
        error = "Invalid end marker.";
        return false;
    }
    // STEP 6: Decode six groups of 7 bits
    for (int i = 0; i < 6; i++){
        string pattern = bits.substr(3 + i * 7, 7);
        bool found = false;
        for (int digit = 0; digit < 10; digit++){
            if (pattern == Code_Table[digit]) {
                number += char('0' + digit);
                found = true;
                break;
            }
        }
        if(!found){
            number = "";
            error = "Invalid digit pattern.";
            return false;
        }
    }
    // STEP 7: Check first digit
    if (number[0] == '0'){
        number = "";
        error = "First digit cannot be zero.";
        return false;
    }
    return true;
}
