#include "reader.h"
#include "../shared.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

static string extractrows(const Mat& pic) {
    string row;
    if (pic.empty()) {
        return row;
    }
    int r = pic.rows / 2;
    for (int c = 0; c < pic.cols; c++) {
        int val = pic.at<uchar>(r, c);
        if (val < 128) {
            row = row + '1';
        }
        else {
            row = row + '0';
        }
    }
    return row;
}

string readBarcodeRow(const string& filename) {
    Mat pic = imread(filename, IMREAD_GRAYSCALE);
    if (pic.empty()) {
        cerr << "Error: could not open '" << filename << "' (file missing or not an image)" << endl;
        return "";
    }
    return extractrows(pic);
}