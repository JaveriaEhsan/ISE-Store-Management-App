#include "decoder.h"
#include "../shared.h"
#include <iostream>
using namespace std;
bool decodeRow(const string& row,string& number,string& error)
{
    number = "";
    error = "";
    // Try possible module widths
    for (int width = 1; width <= 20; width++)
    {
        for (int start = 0; start < row.size(); start++){
            // First module should be black
            if (row[start] != '1')
                continue;
            // Sample centers of all 48 modules
            string bits = "";
            bool valid = true;
            for (int i = 0; i < Total_Bits; i++){
                int position =start + width / 2 + i * width;
                if (position >= row.size()){
                    valid = false;
                    break;
                }
                bits += row[position];
            }
            if (!valid)
                continue;
            // Checking  start marker
            if (bits.substr(0, 3) != Marker)
                continue;
            // Checking end marker
            if (bits.substr(45, 3) != Marker)
                continue;
            string candidate = "";
            // Decoding into six digits
            bool allDigitsValid = true;
            for (int i = 0; i < Digit_Count; i++) {
                string pattern = bits.substr( 3 + i * Bits_Per_Digit,  Bits_Per_Digit);
                bool found = false;
                for (int digit = 0; digit < 10; digit++){
                    if (pattern == Code_Table[digit]){
                        candidate += char('0' + digit);
                        found = true;
                        break;
                    }
                }
                if (!found){
                    allDigitsValid = false;
                    break;
                }
            }
            if (!allDigitsValid){
                continue;
            }
            if (candidate.empty() || candidate[0] == '0'){
                continue;
            }
            number = candidate;
            return true;
        }
    }
    error = "No valid barcode found.";
    return false;
}
