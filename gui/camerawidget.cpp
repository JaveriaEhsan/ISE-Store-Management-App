#include "camerawidget.h"
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

using namespace std;

CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent)
{
    view = new QLabel(this);
    view->setAlignment(Qt::AlignCenter);
    view->setMinimumSize(480, 360);
    view->setStyleSheet("background: black; color: white;");
    view->setText("Camera is off");
    view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

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

// Opens the camera. Each call uses the next driver: DirectShow, Media Foundation, default.
bool CameraWidget::openCamera()
{
    static const int backends[] = { cv::CAP_DSHOW, cv::CAP_MSMF, cv::CAP_ANY };

    if (cap.isOpened()) cap.release();

    bool opened = cap.open(0, backends[reopenCount % 3]);
    if (opened)
    {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    }
    return opened;
}

bool CameraWidget::start()
{
    if (timer->isActive()) return true;

    // try each driver until one opens
    bool opened = false;
    for (reopenCount = 0; reopenCount < 3 && !opened; reopenCount++)
    {
        opened = openCamera();
    }
    reopenCount--;                       // keep the index of the driver that worked

    if (!opened)
    {
        view->setText("Could not open the camera");
        emit cameraError("Could not open the camera. Close other apps that use it.");
        return false;
    }

    lastNumber.clear();
    sameCount = 0;
    missCount = 0;
    blackCount = 0;
    warmup = 15;                         // ignore the first 15 frames
    view->setText("Starting camera...");
    timer->start(30);                    // about 30 frames per second
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

    // let the camera warm up
    if (warmup > 0)
    {
        warmup--;
        return;
    }

    // black picture: the camera is open but gives no real image. Reopen it.
    if (cv::mean(frame)[0] < 3)
    {
        if (++blackCount >= 30)                 // about 1 second of black
        {
            blackCount = 0;
            if (++reopenCount >= 7)             // give up after a few tries
            {
                stop();
                view->setText("The camera gives a black picture");
                emit cameraError("The camera gives a black picture. Close other apps that use it, then reopen this window.");
                return;
            }
            view->setText("Restarting camera...");
            openCamera();                       // releases, then opens with the next driver
            warmup = 15;
        }
        return;
    }
    blackCount = 0;

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
    else if (++missCount > 10)
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

    if (sameCount >= 3)
    {
        QString code = QString::fromStdString(lastNumber);
        stop();
        emit barcodeScanned(code);
    }
}