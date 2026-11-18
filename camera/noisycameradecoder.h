#pragma once

#include <string>

// Decodes one row of '1'/'0' pixels taken from a camera frame.
// On success: returns true and puts the 6-digit number in 'number'.
// On failure: returns false and puts a message in 'error'.
bool decodecameraNoisyRow(const std::string& row,std::string& number,std::string& error);
