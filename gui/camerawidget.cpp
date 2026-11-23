#include "camerawidget.h"
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <opencv2/imgproc.hpp>

using namespace std;

CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent)
{
    view = new QLabel(this);
    view->setAlignment(Qt::AlignCenter);
    view->setMinimumSize(480, 360);
    view->setStyleSheet("background: black; color: white;");
    view->setText("Camera is off");
    view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);  // do not grow with the picture

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::onTick);
}

CameraWidget::~CameraWidget()
{
    stop();
}

bool CameraWidget::start()
{
    if (timer->isActive()) return true;
    
    
    // first camera, Windows driver
       // Windows uses the DirectShow driver; macOS and Linux use OpenCV's default
    #ifdef _WIN32
    const bool cameraOpened = cap.open(0, cv::CAP_DSHOW);
    #else
    const bool cameraOpened = cap.open(0);
    #endif
    if (!cameraOpened)
    {
        view->setText("Could not open the camera");
        emit cameraError("Could not open the camera. Close other apps that use it.");
        return false;
    }

    lastNumber.clear();
    sameCount = 0;
    missCount = 0;
    timer->start(30);                         // about 30 frames per second
    return true;
}

void CameraWidget::stop()
{
    timer->stop();
    if (cap.isOpened()) cap.release();
}

void CameraWidget::onTick()
{
    cv::Mat frame;
    if (!cap.read(frame) || frame.empty())
    {
        stop();
        view->setText("Camera stopped");
        emit cameraError("The camera stopped sending pictures.");
        return;
    }

    // try to read a barcode from this frame
    string number;
    cv::Rect box;
    bool found = scanner.tryDecode(frame, number, &box);

    if (found)
    {
        missCount = 0;
        if (number == lastNumber) sameCount++;
        else { lastNumber = number; sameCount = 1; }
        cv::rectangle(frame, box, cv::Scalar(0, 200, 0), 3);    // green box
    }
    else if (++missCount > 10)     // tolerate a few bad frames, then start over
    {
        lastNumber.clear();
        sameCount = 0;
    }

    // show the frame
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    QImage image(rgb.data, rgb.cols, rgb.rows, (int)rgb.step, QImage::Format_RGB888);
    view->setPixmap(QPixmap::fromImage(image.copy())
                        .scaled(view->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

    // confirmed: same number 3 times in a row
    if (sameCount >= 3)
    {
        QString code = QString::fromStdString(lastNumber);
        stop();
        emit barcodeScanned(code);
    }
}