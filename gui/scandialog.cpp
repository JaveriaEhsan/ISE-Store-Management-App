#include "scandialog.h"
#include "camerawidget.h"
#include "barcode/encoder.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

ScanDialog::ScanDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Scan Product");
    resize(560, 520);

    statusLabel = new QLabel("Hold the barcode in front of the camera, or type it below.", this);
    statusLabel->setWordWrap(true);

    camera = new CameraWidget(this);

    typedEdit = new QLineEdit(this);
    typedEdit->setPlaceholderText("or type the 6-digit barcode");
    typedEdit->setMaxLength(6);
    QPushButton* useButton = new QPushButton("Use typed barcode", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    QHBoxLayout* typedRow = new QHBoxLayout();
    typedRow->addWidget(typedEdit, 1);
    typedRow->addWidget(useButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(statusLabel);
    layout->addWidget(camera, 1);
    layout->addLayout(typedRow);
    layout->addWidget(cancelButton);

    // camera found a barcode: remember it and close
    connect(camera, &CameraWidget::barcodeScanned, this, [this](const QString& code) {
        result = code;
        accept();
    });
    connect(camera, &CameraWidget::cameraError, this, [this](const QString& message) {
        statusLabel->setText(message + "\nYou can still type the barcode below.");
    });

    connect(useButton, &QPushButton::clicked, this, &ScanDialog::onTyped);
    connect(typedEdit, &QLineEdit::returnPressed, this, &ScanDialog::onTyped);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // release the camera whenever the dialog closes (accept, cancel, or the X button)
    connect(this, &QDialog::finished, camera, &CameraWidget::stop);
    QTimer::singleShot(0, camera, &CameraWidget::start);   // start after the dialog is shown
}

void ScanDialog::onTyped()
{
    QString text = typedEdit->text().trimmed();

    if (!isvalidInput(text.toStdString()))
    {
        statusLabel->setText("Enter exactly 6 digits. The first digit cannot be 0.");
        return;
    }

    result = text;
    accept();
}

QString ScanDialog::barcode() const
{
    return result;
}