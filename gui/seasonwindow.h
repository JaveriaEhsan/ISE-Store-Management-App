#ifndef SEASONWINDOW_H
#define SEASONWINDOW_H

#include <QDialog>
#include "../data/ISeasonRepository.h"

class QTableWidget;
class QDateEdit;
class QLineEdit;
class QLabel;

// Manager screen: list seasons, add a season, switch a season on or off
class SeasonWindow : public QDialog
{
    Q_OBJECT

public:
    SeasonWindow(ISeasonRepository& seasons, QWidget* parent = nullptr);

private slots:
    void onAdd();

private:
    void onStatusClicked(int row, bool makeActive);   // the status button of one row
    void updateWarning();                            //when discount is out of range
    bool readPercent(double& value) const;            // false if the box is empty or not a number
    bool percentIsAllowed() const;
    void refresh();                                   // reload the table from the repository

    ISeasonRepository& seasons;

    QTableWidget* table;
    QDateEdit* startEdit;
    QDateEdit* endEdit;
    QLineEdit* percentEdit;
    QLabel* warningLabel;
};

#endif