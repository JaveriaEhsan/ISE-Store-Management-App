#include "camera.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool captureFromCamera(const string& cameraFilename) 
{
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
        if (getWindowProperty("Camera Preview", WND_PROP_VISIBLE) < 1)
        {
            cout << "Camera window closed." << endl;
            break;
        }
        if (key == 32)
        {
            try
            {
                captured = imwrite(cameraFilename, frame);
            }
            catch (const cv::Exception &e)
            {
                cerr << "OpenCV error: " << e.what() << endl;
            }

            if (captured)
                cout << "Captured and saved to " << cameraFilename << endl;
            else
                cerr << "Error: could not save the captured image." << endl;
            break;
        }
        else if (key == 27)
        { // ESC
            cout << "Exiting without capturing." << endl;
            break;
        }
    }

    cap.release(); // Release the camera
    destroyAllWindows(); // Close all OpenCV windows
    return captured; 

}
