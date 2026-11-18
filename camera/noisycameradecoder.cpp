
#include "noisycameradecoder.h"
#include "../shared.h"

#include <algorithm>
#include <string>

using namespace std;

bool decodecameraNoisyRow(
    const string& row,
    string& number,
    string& error)
{
    number.clear();
    error.clear();

    const int rowSize = static_cast<int>(row.size());

    if (rowSize < Total_Bits)
    {
        error = "Row too short.";
        return false;
    }

    // Avoid testing widths that cannot fit all modules.
    const int maxWidth = min(20, rowSize / Total_Bits);

    string candidate;
    candidate.reserve(Digit_Count);

    // Try possible module widths.
    for (int width = 1; width <= maxWidth; ++width)
    {
        const int halfWidth = width / 2;

        // Only scan positions where all modules can fit.
        const int lastStart =
            rowSize - 1 - halfWidth - (Total_Bits - 1) * width;

        for (int start = 0; start <= lastStart; ++start)
        {
            if (row[start] != '1')
                continue;

            // Check start marker directly.
            bool markerValid = true;

            for (int j = 0; j < 3; ++j)
            {
                int pos = start + halfWidth + j * width;

                if (row[pos] != Marker[j])
                {
                    markerValid = false;
                    break;
                }
            }

            if (!markerValid)
                continue;

            // Check end marker directly.
            for (int j = 0; j < 3; ++j)
            {
                int module = 45 + j;
                int pos = start + halfWidth + module * width;

                if (row[pos] != Marker[j])
                {
                    markerValid = false;
                    break;
                }
            }

            if (!markerValid)
                continue;

            candidate.clear();

            // Decode six digits without creating substrings.
            bool valid = true;

            for (int i = 0; i < Digit_Count; ++i)
            {
                const int firstModule =
                    3 + i * Bits_Per_Digit;

                bool digitFound = false;

                for (int digit = 0; digit < 10; ++digit)
                {
                    bool match = true;

                    for (int j = 0; j < Bits_Per_Digit; ++j)
                    {
                        int module = firstModule + j;

                        int pos =
                            start + halfWidth + module * width;

                        if (row[pos] != Code_Table[digit][j])
                        {
                            match = false;
                            break;
                        }
                    }

                    if (match)
                    {
                        candidate.push_back(
                            static_cast<char>('0' + digit));

                        digitFound = true;
                        break;
                    }
                }

                if (!digitFound)
                {
                    valid = false;
                    break;
                }
            }

            if (!valid)
                continue;

            if (candidate.empty() || candidate[0] == '0')
                continue;

            number = candidate;
            return true;
        }
    }

    error = "No valid barcode found.";
    return false;
}
