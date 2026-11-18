#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QDialog>
#include <QString>

class CameraWidget;
class QLineEdit;
class QLabel;

// Scans a barcode with the camera, or lets the user type it.
// After exec() == Accepted, barcode() returns the 6-digit number.
class ScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanDialog(QWidget* parent = nullptr);
    QString barcode() const;

private slots:
    void onTyped();

private:
    CameraWidget* camera;
    QLineEdit* typedEdit;
    QLabel* statusLabel;
    QString result;
};

#endif