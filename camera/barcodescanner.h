#pragma once
#include <opencv2/core.hpp>
#include <string>

class BarcodeScanner {
public:
    // Decodes one camera frame. Returns true and fills 'number' on success.
    // 'box' (optional) receives the barcode area, for drawing a rectangle.
    bool tryDecode(const cv::Mat& frame, std::string& number,
                   cv::Rect* box = nullptr) const;
};