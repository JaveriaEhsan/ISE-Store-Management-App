#include "noisycameradecoder.h"
#include "../shared.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

namespace {

constexpr int MarkerBits    = 3;    // modules in the start / end marker
constexpr int StepsPerPixel = 20;   // width resolution: 0.05 px
constexpr int MaxModulePx   = 20;   // widest module we try
constexpr int    DigitValues        = 10;     // digits 0 to 9
constexpr double SampleWindowRatio  = 0.25;   // sample window = 1/4 of module width
constexpr int    MaxTransitionsMult = 3;      // allowed transitions = 3 x Total_Bits

// Majority vote over a small window around the module center.
// 'ones' is a prefix-sum array: ones[i] = number of '1' pixels in row[0, i).
// 'width' is fractional. Caller guarantees the window is inside the row.
inline char sampleModule(const vector<int>& ones, int start, int module, double width)
{
    const int center = start + static_cast<int>((module + 0.5) * width);
    const int half   = static_cast<int>(width * SampleWindowRatio);

    const int count = ones[center + half + 1] - ones[center - half];
    const int total = 2 * half + 1;
    return (count * 2 > total) ? '1' : '0';
}

bool decodeAt(const vector<int>& ones, int start, double width, string& candidate)
{
    for (int j = 0; j < MarkerBits; ++j)
    {
        if (sampleModule(ones, start, j, width) != Marker[j])
            return false;
        if (sampleModule(ones, start, Total_Bits - MarkerBits + j, width) != Marker[j])
            return false;
    }

    candidate.clear();

    for (int i = 0; i < Digit_Count; ++i)
    {
        const int firstModule = MarkerBits + i * Bits_Per_Digit;
        bool found = false;

        for (int digit = 0; digit < DigitValues && !found; ++digit)
        {
            bool match = true;
            for (int j = 0; j < Bits_Per_Digit; ++j)
            {
                if (sampleModule(ones, start, firstModule + j, width) != Code_Table[digit][j])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                candidate.push_back(static_cast<char>('0' + digit));
                found = true;
            }
        }

        if (!found)
            return false;
    }

    return !candidate.empty() && candidate[0] != '0';   // IDs never start with 0
}

} // namespace

bool decodecameraNoisyRow(const string& row, string& number, string& error)
{
    number.clear();
    error.clear();

    const int rowSize = static_cast<int>(row.size());
    if (rowSize < Total_Bits)
    {
        error = "Row too short.";
        return false;
    }

    vector<int> ones(rowSize + 1, 0);
    for (int i = 0; i < rowSize; ++i)
        ones[i + 1] = ones[i] + (row[i] == '1');

    // Cheap reject: a real barcode row has at most ~Total_Bits transitions.
    // A generous bound keeps blur / speckle on real codes from tripping it.
    int transitions = 0;
    for (int i = 1; i < rowSize; ++i)
        transitions += (row[i] != row[i - 1]);
    if (transitions > MaxTransitionsMult * Total_Bits)
    {
        error = "Too many transitions for a barcode.";
        return false;
    }

    const int maxStep = min(MaxModulePx * StepsPerPixel,
                            rowSize * StepsPerPixel / Total_Bits);

    string candidate;
    candidate.reserve(Digit_Count);

    for (int step = StepsPerPixel; step <= maxStep; ++step)
    {
        const double width = static_cast<double>(step) / StepsPerPixel;
        const int half = static_cast<int>(width * SampleWindowRatio);

        // Last start where the final module's whole sample window is inside the row.
        const int lastStart =
            rowSize - 1 - static_cast<int>((Total_Bits - 0.5) * width) - half;

        for (int start = 0; start <= lastStart; ++start)
        {
            if (row[start] != '1')
                continue;

            if (decodeAt(ones, start, width, candidate))
            {
                number = candidate;
                return true;
            }
        }
    }

    error = "No valid barcode found.";
    return false;
}
