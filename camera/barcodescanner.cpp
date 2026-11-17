#include "barcodescanner.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include<iostream>
#include <string>
#include <vector>
using namespace std;
using namespace cv;

#include "noisycameradecoder.h"      // CHANGED: same camera/ folder

static Mat preprocessFrame(const Mat& frame)
{
    Mat gray, blurred, binary;
    // CHANGED: accepts 1, 3 or 4 channel frames (yours: BGR2GRAY only)
    
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
    equalizeHist(blurred, blurred);
    threshold(blurred, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
    return binary;
}

static vector<Rect> detectBarcodeRegions(const Mat& image)   // unchanged
{
    Mat gradient, detected;
    Sobel(image, gradient, CV_16S, 1, 0, 3);
    convertScaleAbs(gradient, detected);
    threshold(detected,detected,0,255,THRESH_BINARY | THRESH_OTSU);

    int kernelWidth = max(15, image.cols / 30);
    Mat kernel = getStructuringElement(MORPH_RECT, Size(kernelWidth, 3));
    morphologyEx(detected, detected, MORPH_CLOSE, kernel);
    Mat dilationKernel = getStructuringElement(
    MORPH_RECT,
    Size(5, 3)
);

dilate(detected, detected, dilationKernel);

    vector<vector<Point>> contours;
    findContours(detected, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Rect> regions;
    regions.reserve(contours.size());
    for (const auto& contour : contours) {
        Rect box = boundingRect(contour);
        if (box.width < 60) continue;
        if (box.width < box.height * 2) continue;
        if (box.area() < 1000) continue;

        int mx = box.width / 20;
        int my = box.height / 5;
        box.x = max(0, box.x - mx);
        box.y = max(0, box.y - my);
        box.width  = min(image.cols - box.x, box.width + 2 * mx);
        box.height = min(image.rows - box.y, box.height + 2 * my);
        regions.push_back(box);
    }
    sort(regions.begin(),regions.end(),[](const Rect& a, const Rect& b){
        return a.area() > b.area();
    }
);
    return regions;
}

static bool decodeRegion(const Mat& region, string& decodedNumber)  // unchanged
{
    if (region.empty()) return false;
    if(region.type() != CV_8UC1)
        return false;

    static const double positions[] = {0.30, 0.40, 0.50, 0.60, 0.70};
    for (double p : positions) {
        int r = min(region.rows - 1, static_cast<int>(region.rows * p));
        string row;
        row.resize(region.cols);
        const uchar* pixels = region.ptr<uchar>(r);
        for (int c = 0; c < region.cols; c++)
            row[c] = (pixels[c] < 128) ? '1' : '0';
        string candidate, error;
        if (decodecameraNoisyRow(row, candidate, error)) {
            decodedNumber = candidate;
            return true;
        }
    }
    return false;
}

// CHANGED: was processFrame(frame, number, Rect& barcodeBox)
bool BarcodeScanner::tryDecode(const Mat& frame, string& number, Rect* box) const
{
    number.clear();
    if (box)
    *box=Rect();
    if (frame.empty()) return false;

    Mat binary = preprocessFrame(frame);
    if(binary.empty()) 
    return false;
    for (const Rect& r : detectBarcodeRegions(binary)) {
        if(r.empty())
        continue;
        if (decodeRegion(binary(r), number)) {
            if (box) *box = r;                       // CHANGED: optional pointer
            return true;
        }
    }

    // Fallback: scan the complete frame
    if (decodeRegion(binary, number)) {
        if (box) *box = Rect(0, 0, frame.cols, frame.rows);
        return true;
    }
    return false;
}
