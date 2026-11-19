#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <string>
#include <opencv2/videoio.hpp>
#include "camera/barcodescanner.h"      // fix the folder if different

class QLabel;
class QTimer;

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget();

public slots:
    bool start();     // opens the camera and starts scanning
    void stop();      // stops scanning and releases the camera

signals:
    void barcodeScanned(const QString& barcode);   // same number 3 times in a row
    void cameraError(const QString& message);

private slots:
    void onTick();

private:
    cv::VideoCapture cap;
    BarcodeScanner scanner;
    QTimer* timer;
    QLabel* view;

    std::string lastNumber;
    int sameCount = 0;
    int missCount = 0;
};

#endif