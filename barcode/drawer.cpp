#include "drawer.h"
#include "../shared.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

bool isValidBitString(const string& bits)
{
    if (bits.size() != 48)
    {
        return false;
    }

    for (int i = 0; i < bits.size(); i++)
    {
        if (bits[i] != '0' && bits[i] != '1')
        {
            return false;
        }
    }

    return true;
}

void printConsole(const string& bits)
{
    string bar = "\xDB";

    for (int row = 0; row < 5; row++)
    {
        for (int i = 0; i < bits.size(); i++)
        {
            if (bits[i] == '1')
            {
                cout << bar;
            }
            else
            {
                cout << "  ";
            }
        }
        cout << endl;
    }
}


bool createBarcodeImage(const string& bits, const string& filename)
{
    if (!isValidBitString(bits))
    {
        cerr << "Error: Invalid barcode bit string!" << endl;
        return false;
    }

    int width = (bits.size() + 2 * Quiet_Zone) * Bar_Width;
    Mat image(Image_Height, width, CV_8UC3, Scalar(255, 255, 255));
    for (int i = 0; i < bits.size(); i++)
    {
        if (bits[i] == '1')
        {
            int x = (Quiet_Zone + i) * Bar_Width;
            rectangle(image, Point(x, 0),
                Point(x + Bar_Width - 1, Image_Height - 1),
                Scalar(0, 0, 0), FILLED);
        }
    }
    cout << "Saved " << filename << " successfully" << endl;

    try
    {
        if (!imwrite(filename, image))
        {
            cerr << "Error: Could not save " << filename << endl;
            return false;
        }
    }
    catch (const cv::Exception& e)
    {
        cerr << "OpenCV error: " << e.what() << endl;
        return false;
    }

    cout << "Saved " << filename << " successfully" << endl;
    imshow("Generated Barcode", image);
    waitKey(1500);
    destroyAllWindows();
    return true;

    
}