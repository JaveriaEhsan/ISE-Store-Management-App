#include "camera.h"
#include "decoder.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;


bool captureFromCamera(const string& cameraFilename)
{
    string decodedNumber;
    return captureFromCamera(cameraFilename, decodedNumber);
}

// Convert camera frame into a binary image
static Mat preprocessFrame(const Mat& frame)
{
    Mat gray, blurred, binary;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(3, 3), 0);
    equalizeHist(blurred, blurred);
    threshold(blurred,binary,0,255,THRESH_BINARY | THRESH_OTSU);
    return binary;
}

// Find possible barcode regions
static vector<Rect> detectBarcodeRegions(const Mat& image){
    Mat gradient, absGradient, detected;
    Sobel(image, gradient, CV_32F, 1, 0, 3);
    convertScaleAbs(gradient, absGradient);
    threshold(absGradient,detected,0,255,THRESH_BINARY | THRESH_OTSU);
    int kernelWidth = max(15, image.cols / 30);
    Mat kernel = getStructuringElement(MORPH_RECT,Size(kernelWidth, 3));
    morphologyEx(detected,detected,MORPH_CLOSE,kernel);
    dilate(detected,detected,getStructuringElement(MORPH_RECT,Size(5, 3)));
    vector<vector<Point>> contours;
    findContours(detected,contours, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    vector<Rect> regions;
    for (const auto& contour : contours){
        Rect box = boundingRect(contour);
        if (box.width < 60)
            continue;
        if (box.width < box.height * 2)
            continue;
        if (box.area() < 1000)
            continue;
        int mx = box.width / 20;
        int my = box.height / 5;
        box.x = max(0, box.x - mx);
        box.y = max(0, box.y - my);
        box.width = min(image.cols - box.x,box.width + 2 * mx);
        box.height = min(image.rows - box.y,box.height + 2 * my);
        regions.push_back(box);
    }
    sort(regions.begin(),regions.end(),[](const Rect& a, const Rect& b){
            return a.area() > b.area();
        }
    );
    return regions;
}

// Try decoding several horizontal rows
static bool decodeRegion(const Mat& region,string& decodedNumber){
    if (region.empty())
        return false;
    double positions[] = {0.30, 0.40, 0.50, 0.60, 0.70};
    for (double p : positions){
        int r = min(region.rows - 1,static_cast<int>(region.rows * p));
        string row;
        for (int c = 0; c < region.cols; c++){
            row += region.at<uchar>(r, c) < 128? '1' : '0';
        }
        string candidate;
        string error;
        if (decodeRow(row, candidate, error)){
            decodedNumber = candidate;
            return true;
        }
    }
    return false;
}

// Process one frame
static bool processFrame(const Mat& frame, string& number,Rect& barcodeBox){
    Mat binary = preprocessFrame(frame);
    vector<Rect> regions =detectBarcodeRegions(binary);
    for (const Rect& box : regions) {
        Mat roi = binary(box);
        if (decodeRegion(roi, number)){
            barcodeBox = box;
            return true;
        }
    }

    // Fallback: scan the complete frame
    if (decodeRegion(binary, number)) {
        barcodeBox = Rect(0, 0, frame.cols, frame.rows);
        return true;
    }
    return false;
}

// Automatic camera scanning
bool captureFromCamera(const string& cameraFilename,string& decodedNumber)
{
    decodedNumber = "";
    VideoCapture cap(0);
    if (!cap.isOpened()){
        cerr << "Error: Could not open camera." << endl;
        return false;
    }
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    cout << "Scanning automatically..." << endl;
    cout << "Hold barcode horizontally in front of camera."<< endl;
    cout << "Press ESC to cancel." << endl;
    Mat frame;
    string previousNumber = "";
    int stableCount = 0;

    while (true){
        cap >> frame;
        if (frame.empty())
            continue;
        string currentNumber;
        Rect barcodeBox;
        bool found = processFrame(frame,currentNumber,barcodeBox);

        if (found){
            rectangle(frame,barcodeBox,Scalar(0, 255, 0),2);

            putText(frame,"Barcode: " + currentNumber,Point(20, 35),FONT_HERSHEY_SIMPLEX,0.8,Scalar(0, 255, 0),2);
            if (currentNumber == previousNumber){
                stableCount++;
            }
            else{
                previousNumber = currentNumber;
                stableCount = 1;
            }
            cout << "\rDetected: " << currentNumber << " Confirmation: " << stableCount << "/3" << flush;
            if (stableCount >= 3){
                decodedNumber = currentNumber;
                // Save original frame
                bool saved = imwrite(cameraFilename,frame);
                cap.release();
                destroyAllWindows();
                if (!saved){
                    decodedNumber = "";
                    cerr << "\nError saving image." << endl;
                    return false;
                }
                cout << "\nBarcode captured successfully!" << endl;
                cout << "Decoded number: " << decodedNumber << endl;
                return true;
            }
        }
        else{
            previousNumber = "";
            stableCount = 0;
        }
        imshow("Automatic Barcode Scanner", frame);
        if (waitKey(1) == 27)
            break;
    }

    cap.release();
    destroyAllWindows();

    return false;
}