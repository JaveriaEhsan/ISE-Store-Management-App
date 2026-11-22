#include "barcodescanner.h"
#include "noisycameradecoder.h"

#include <opencv2/geometry/2d.hpp>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
using namespace cv;

namespace {

    constexpr int    MinCloseKernel  = 15;
constexpr int    CloseKernelDiv  = 30;
constexpr int    MinRegionWidth  = 60;
constexpr int    MinAspectRatio  = 2;     // width must be >= 2 x height
constexpr int    MinRegionArea   = 1000;
constexpr int    BlackThreshold  = 128;   // darker than this = '1'
constexpr int    FallbackRowStep = 4;     // last resort: every 4th row
constexpr double RowPositions[]  = { 0.25, 0.32, 0.40, 0.46, 0.50, 0.54, 0.60, 0.68, 0.75 };


Mat preprocessFrame(const Mat& frame)
{
    Mat gray, blurred, binary;

    if (frame.empty())
        return binary;

    if (frame.channels() == 3)
        cvtColor(frame, gray, COLOR_BGR2GRAY);
    else if (frame.channels() == 4)
        cvtColor(frame, gray, COLOR_BGRA2GRAY);
    else if (frame.channels() == 1)
        gray = frame;
    else
        return binary;

    if (gray.depth() != CV_8U)
        gray.convertTo(gray, CV_8U);

    GaussianBlur(gray, blurred, Size(3, 3), 0);

    // equalizeHist removed: it destroyed blurred bars before Otsu ran.
    threshold(blurred, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
    return binary;
}

vector<Rect> detectBarcodeRegions(const Mat& image)
{
    Mat gradient, detected;
    Sobel(image, gradient, CV_16S, 1, 0, 3);
    convertScaleAbs(gradient, detected);
    threshold(detected, detected, 0, 255, THRESH_BINARY | THRESH_OTSU);

    const int kernelWidth = max(MinCloseKernel, image.cols / CloseKernelDiv);
    Mat closeKernel = getStructuringElement(MORPH_RECT, Size(kernelWidth, 3));
    morphologyEx(detected, detected, MORPH_CLOSE, closeKernel);

    Mat dilateKernel = getStructuringElement(MORPH_RECT, Size(5, 3));
    dilate(detected, detected, dilateKernel);

    vector<vector<Point>> contours;
    findContours(detected, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Rect> regions;
    regions.reserve(contours.size());

    for (const auto& contour : contours)
    {
        Rect box = boundingRect(contour);
       if (box.width < MinRegionWidth) continue;
       if (box.width < box.height * MinAspectRatio) continue;
       if (box.area() < MinRegionArea) continue;
        const int mx = box.width / 20;
        const int my = box.height / 5;
        box.x = max(0, box.x - mx);
        box.y = max(0, box.y - my);
        box.width  = min(image.cols - box.x, box.width + 2 * mx);
        box.height = min(image.rows - box.y, box.height + 2 * my);
        regions.push_back(box);
    }

    sort(regions.begin(), regions.end(),
         [](const Rect& a, const Rect& b) { return a.area() > b.area(); });
    return regions;
}

// Turn one row of a binary image into a '1'/'0' string and try to decode it.
bool decodeBinaryRow(const Mat& img, int r, string& decodedNumber)
{
    string row(img.cols, '0');
    const uchar* pixels = img.ptr<uchar>(r);
    for (int c = 0; c < img.cols; ++c)
       row[c] = (pixels[c] < BlackThreshold) ? '1' : '0';
    string candidate, error;
    if (decodecameraNoisyRow(row, candidate, error))
    {
        decodedNumber = candidate;
        return true;
    }
    return false;
}

// Try several rows through the middle of a region.
bool decodeRegion(const Mat& region, string& decodedNumber)
{
    if (region.empty() || region.type() != CV_8UC1)
        return false;

    
    for (double p : RowPositions)
    {
        const int r = min(region.rows - 1, static_cast<int>(region.rows * p));
        if (decodeBinaryRow(region, r, decodedNumber))
            return true;
    }
    return false;
}

} // namespace

bool BarcodeScanner::tryDecode(const Mat& frame, string& number, Rect* box) const
{
    number.clear();
    if (box)
        *box = Rect();

    if (frame.empty())
        return false;

    Mat binary = preprocessFrame(frame);
    if (binary.empty())
        return false;

    // 1. Detected regions, largest first.
    for (const Rect& r : detectBarcodeRegions(binary))
    {
        if (r.empty())
            continue;

        if (decodeRegion(binary(r), number))
        {
            if (box)
                *box = r;
            return true;
        }
    }

    // 2. Fallback: middle rows of the whole frame.
    if (decodeRegion(binary, number))
    {
        if (box)
            *box = Rect(0, 0, frame.cols, frame.rows);
        return true;
    }

    // 3. Last resort: every 4th row; accept only when two rows agree.
    {
        string first;
        int firstRow = -1;
        for (int r = 0; r < binary.rows; r += FallbackRowStep)
        {
            string cand;
            if (!decodeBinaryRow(binary, r, cand))
                continue;

            if (firstRow < 0)
            {
                first = cand;
                firstRow = r;
            }
            else if (cand == first)
            {
                number = cand;
                if (box)
                    *box = Rect(0, firstRow, frame.cols, 1);
                return true;
            }
        }
    }

    return false;
}
