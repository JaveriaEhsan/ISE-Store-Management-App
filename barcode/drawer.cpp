
#include "drawer.h"
#include "../shared.h"

#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;
using namespace cv;

static bool isValidBitString(const string& bits)
{
    if (bits.size() != Total_Bits)
        return false;

    for (char bit : bits)
    {
        if (bit != '0' && bit != '1')
            return false;
    }

    return true;
}

Mat buildBarcodeImage(const string& bits)
{
    if (!isValidBitString(bits))
        return Mat();

    int width = (bits.size() + 2 * Quiet_Zone) * Bar_Width;

    Mat image(Image_Height, width, CV_8UC3,
              Scalar(255, 255, 255));

    for (int i = 0; i < bits.size(); i++)
    {
        if (bits[i] == '1')
        {
            int x = (Quiet_Zone + i) * Bar_Width;

            rectangle(image,
                      Point(x, 0),
                      Point(x + Bar_Width - 1, Image_Height - 1),
                      Scalar(0, 0, 0),
                      FILLED);
        }
    }

    return image;
}

bool saveBarcodeImage(const Mat& image, const string& filename)
{
    if (image.empty())
        return false;

    try
    {
        filesystem::path path(filename);

        if (path.has_parent_path())
            filesystem::create_directories(path.parent_path());

        return imwrite(filename, image);
    }
    catch (const exception&)
    {
        return false;
    }
}

string makeBarcodeFilename(const string& number)
{
    return "barcodes/" + number + ".png";
}
