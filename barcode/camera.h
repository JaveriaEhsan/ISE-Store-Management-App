#ifndef CAMERA_H
#define CAMERA_H

#include <string>
using namespace std;

// Opens the camera, shows a live preview, and saves one frame to cameraFilename.
// Returns true if a picture was captured.
bool captureFromCamera(const std::string& cameraFilename);

#endif