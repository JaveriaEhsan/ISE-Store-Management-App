#include "camera.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool captureFromCamera(const string& cameraFilename) 
{
    // VideoCapture cap(0, CAP_DSHOW); // CAP_DSHOW tells OpenCV to use the Windows DirectShow driver
    VideoCapture cap(0); // Open the default camera (camera index 0)
    if (!cap.isOpened()) 
    {
        cerr << "Error: Could not open the camera." << endl;
        return false;
    }

    cout << "Hold the barcode in front of the camera." << endl;
    cout << "Click the camera window, then press SPACE to capture or ESC to cancel." << endl;

    Mat frame;
    bool captured = false;

    while (true) 
    {
        cap >> frame; // Capture a new frame from the camera
        if (frame.empty()) 
        {
            cerr << "Error: Could not read a frame from the camera." << endl;
            break;
        }

        imshow("Camera Preview", frame); // Show the live preview

        int key = waitKey(30); 
        if (key == 32) { // SPACE 
            captured = imwrite(cameraFilename, frame); // Save the captured frame to the specified filename
            cout << "Captured and saved to " << cameraFilename << endl;
            break;
        } else if (key == 27) { // ESC 
            cout << "Exiting without capturing." << endl;
            break;
        }
    }

    cap.release(); // Release the camera
    destroyAllWindows(); // Close all OpenCV windows
    return captured; 

}
