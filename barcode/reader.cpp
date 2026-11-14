#include "reader.h"
#include "../shared.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
using namespace std;
using namespace cv;
static string extractrows(const Mat& pic,int r) {
    string row;
    if (pic.empty()) {
        return row;
    }
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
    vector<string> rows;
    int step = max(1, pic.rows / 100);
    for (int r = 0; r < pic.rows; r += step) {
        rows.push_back(extractrows(pic, r));
    }
    if (rows.empty())
        return "";
    return rows[rows.size() / 2];
}