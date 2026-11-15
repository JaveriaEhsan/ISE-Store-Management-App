
#ifndef DRAWER_H
#define DRAWER_H

#include <string>
#include <opencv2/core.hpp>

using namespace std;

// Build barcode image and return it as cv::Mat
cv::Mat buildBarcodeImage(const string& bits);

// Save an already-built barcode image
bool saveBarcodeImage(const cv::Mat& image, const string& filename);

// Generate barcode filename
string makeBarcodeFilename(const string& number);

#endif
