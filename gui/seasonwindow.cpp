
#include "seasonwindow.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDate>
#include <QCalendarWidget>
#include <string>
#include <vector>
using namespace std;

// The file keeps dates as 2026-11-21, the screen shows 21-11-2026
static const char* FILE_DATE = "yyyy-MM-dd";
static const char* SCREEN_DATE = "dd-MM-yyyy";

// A season discount should stay between these two numbers
static const double MIN_SEASON_PERCENT = 5.0;
static const double MAX_SEASON_PERCENT = 70.0;

static QString toScreen(const string& fileDate)
{
    QDate d = QDate::fromString(QString::fromStdString(fileDate), FILE_DATE);
    return d.toString(SCREEN_DATE);
}

// A table cell with the text in the middle
static QTableWidgetItem* makeCell(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

// Empty text if the discount is fine, otherwise a short reason
static QString percentProblem(double value)
{
    if (value < MIN_SEASON_PERCENT)
    {
        return QString("The discount is too low. The minimum is %1%.")
            .arg(MIN_SEASON_PERCENT, 0, 'f', 0);
    }

    if (value > MAX_SEASON_PERCENT)
    {
        return QString("The discount is too high. The maximum is %1%.")
            .arg(MAX_SEASON_PERCENT, 0, 'f', 0);
    }

    return "";
}

// Lists the active seasons whose dates touch start..end (file dates, YYYY-MM-DD).
// skipRow is a row to ignore (the season itself), or -1.
static QString overlapText(const vector<Season>& all, const string& start,
                           const string& end, int skipRow)
{
    QString text;

    for (int i = 0; i < (int)all.size(); i++)
    {
        if (i == skipRow || !all[i].isActive)
        {
            continue;
        }

        // two date ranges overlap when each one starts before the other ends
        if (start <= all[i].endDate && all[i].startDate <= end)
        {
            text += toScreen(all[i].startDate) + " to " + toScreen(all[i].endDate)
                + "  (" + QString::number(all[i].discountPercent, 'f', 2) + "%)\n";
        }
    }

    return text;
}

namespace
{
    QString primaryButtonStyle()
    {
        return "QPushButton {"
            "background-color: #1761D1; color: white;"
            "font-size: 17px; font-weight: bold; border-radius: 9px;"
            "}"
            "QPushButton:hover { background-color: #104AA9; }";
    }

    QString inputStyle()
    {
        return "QDateEdit, QLineEdit {"
            "background-color: #F4F6FA;"
            "border: 1px solid #D9DEE8;"
            "border-radius: 8px;"
            "padding-left: 12px;"
            "font-size: 16px;"
            "color: #172342;"
            "}"
            "QDateEdit:focus, QLineEdit:focus { border: 2px solid #1761D1; }";
    }

    QString panelStyle()
    {
        return "QFrame#panel {"
            "background-color: white;"
            "border: 1px solid #E2E6EF;"
            "border-radius: 14px;"
            "}"
            "QLabel { background: transparent; border: none; }";
    }

    QString tableStyle()
    {
        return "QTableWidget {"
            "background-color: white;"
            "border: 1px solid #E2E6EF;"
            "border-radius: 8px;"
            "font-size: 15px;"
            "color: #172342;"
            "}"
            "QTableWidget::item { padding: 6px; }"
            "QHeaderView::section {"
            "background-color: #F4F6FA;"
            "color: #34415C;"
            "font-size: 14px; font-weight: bold;"
            "border: none; border-bottom: 1px solid #E2E6EF;"
            "padding: 8px;"
            "}";
    }

    QString headingStyle()
    {
        return "font-size: 20px; font-weight: bold; color: #172342;";
    }
} // namespace


// Window

SeasonWindow::SeasonWindow(ISeasonRepository& seasons, QWidget* parent)
    : QDialog(parent), seasons(seasons)
{
    setWindowTitle("Seasonal Discounts");

    // a dialog has no maximize or minimize button by default, so add them
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

    setMinimumSize(700, 560);
    resize(800, 700);
    setStyleSheet("QDialog { background-color: #F4F6FA; }");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    //  header
    QWidget* header = new QWidget;
    header->setFixedHeight(90);
    header->setStyleSheet("background-color: #07143E;");

    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(28, 14, 28, 12);
    headerLayout->setSpacing(4);

    QLabel* title = new QLabel("Seasonal Discounts");
    title->setStyleSheet("color: white; font-size: 26px; font-weight: bold;");

    QLabel* subtitle = new QLabel("Add a season and switch it on or off");
    subtitle->setStyleSheet("color: white; font-size: 15px;");

    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    root->addWidget(header);

    //  page body 
    QWidget* body = new QWidget;
    QVBoxLayout* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(28, 14, 28, 14);
    bodyLayout->setSpacing(12);

    // card 1: the seasons
    QFrame* listCard = new QFrame;
    listCard->setObjectName("panel");
    listCard->setStyleSheet(panelStyle());

    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(22, 18, 22, 18);
    listLayout->setSpacing(10);

    QLabel* listHeading = new QLabel("All seasons");
    listHeading->setStyleSheet(headingStyle());

    QLabel* hint = new QLabel("Click the status button to switch a season on or off.");
    hint->setStyleSheet("font-size: 13px; color: #7A8294;");

    // heading on the left, hint on the right, so nothing sits under the table
    QHBoxLayout* headingRow = new QHBoxLayout;
    headingRow->addWidget(listHeading);
    headingRow->addStretch();
    headingRow->addWidget(hint);

    table = new QTableWidget(0, 4);
    table->setHorizontalHeaderLabels({ "Start date", "End date", "Discount %", "Status" });
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(44);
    table->setShowGrid(false);
    table->setMinimumHeight(130);
    table->setStyleSheet(tableStyle());

    listLayout->addLayout(headingRow);
    listLayout->addWidget(table, 1);

    // card 2: add a season 
    QFrame* addCard = new QFrame;
    addCard->setObjectName("panel");
    addCard->setStyleSheet(panelStyle());

    QVBoxLayout* addLayout = new QVBoxLayout(addCard);
    addLayout->setContentsMargins(22, 14, 22, 14);
    addLayout->setSpacing(8);

    QLabel* addHeading = new QLabel("Add a new season");
    addHeading->setStyleSheet(headingStyle());

    startEdit = new QDateEdit(QDate::currentDate());
    startEdit->setCalendarPopup(true);
    startEdit->setDisplayFormat(SCREEN_DATE);
    startEdit->setMinimumHeight(42);
    startEdit->setStyleSheet(inputStyle());

    endEdit = new QDateEdit(QDate::currentDate());
    endEdit->setCalendarPopup(true);
    endEdit->setDisplayFormat(SCREEN_DATE);
    endEdit->setMinimumHeight(42);
    endEdit->setMinimumDate(startEdit->date());
    endEdit->setStyleSheet(inputStyle());

    // calendar style
    const QString calendarStyle =
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #07143E; }"
        "QCalendarWidget QToolButton { color: white; background-color: transparent; font-weight: bold; }"
        "QCalendarWidget QToolButton:hover { background-color: #1B3485; }"
        "QCalendarWidget QMenu { color: #172342; background-color: white; }"
        "QCalendarWidget QSpinBox { color: white; background-color: #07143E; }"
        "QCalendarWidget QAbstractItemView:enabled {"
        "  selection-background-color: #1761D1; selection-color: white; }";

    startEdit->calendarWidget()->setStyleSheet(calendarStyle);
    endEdit->calendarWidget()->setStyleSheet(calendarStyle);

    // the discount is a plain text box, so the number is always visible
    percentEdit = new QLineEdit;
    percentEdit->setPlaceholderText("Discount in percent, for example 10");
    percentEdit->setMaxLength(6);
    percentEdit->setMinimumHeight(42);

    QDoubleValidator* validator = new QDoubleValidator(0.0, 100.0, 2, percentEdit);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(QLocale::c());
    percentEdit->setValidator(validator);
    percentEdit->setStyleSheet(inputStyle());

    // warning label (hidden so no red text appears)
    warningLabel = new QLabel;
    warningLabel->setMinimumHeight(20);
    warningLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #D93025;");
    warningLabel->hide();

    QFormLayout* form = new QFormLayout();
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(10);
    form->addRow("Start date:", startEdit);
    form->addRow("End date:", endEdit);
    form->addRow("Discount (%):", percentEdit);

    QPushButton* addButton = new QPushButton("Add Season");
    addButton->setMinimumHeight(44);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet(primaryButtonStyle());

    addLayout->addWidget(addHeading);
    addLayout->addLayout(form);
    addLayout->addWidget(warningLabel);
    addLayout->addWidget(addButton);

    bodyLayout->addWidget(listCard, 1);
    bodyLayout->addWidget(addCard);
    root->addWidget(body, 1);

    connect(addButton, &QPushButton::clicked, this, &SeasonWindow::onAdd);

    connect(percentEdit, &QLineEdit::textChanged, this, [this](const QString&)
    {
        updateWarning();
    });

    // when the start date changes, the end date cannot stay before it
    connect(startEdit, &QDateEdit::dateChanged, this, [this](const QDate& start)
    {
        endEdit->setMinimumDate(start);
        if (endEdit->date() < start)
        {
            endEdit->setDate(start);
        }
    });

    refresh();
}

// Reads the number from the box. False if it is empty or not a number.
bool SeasonWindow::readPercent(double& value) const
{
    bool ok = false;
    value = QLocale::c().toDouble(percentEdit->text().trimmed(), &ok);
    return ok;
}

bool SeasonWindow::percentIsAllowed() const
{
    double value = 0;
    if (!readPercent(value))
    {
        return false;
    }
    return percentProblem(value).isEmpty();
}

void SeasonWindow::updateWarning()
{
    double value = 0;

    // an empty box shows no warning, the manager has not typed yet
    if (!readPercent(value))
    {
        warningLabel->setText("");
        return;
    }

    warningLabel->setText(percentProblem(value));
}

// Read the seasons again and fill the table
void SeasonWindow::refresh()
{
    seasons.load();
    vector<Season> all = seasons.getAll();

    table->setRowCount((int)all.size());

    for (int row = 0; row < (int)all.size(); row++)
    {
        const Season& s = all[row];

        // all three columns have their text in the middle
        table->setItem(row, 0, makeCell(toScreen(s.startDate)));
        table->setItem(row, 1, makeCell(toScreen(s.endDate)));
        table->setItem(row, 2, makeCell(QString::number(s.discountPercent, 'f', 2)));
    }

    string today = QDate::currentDate().toString(FILE_DATE).toStdString();

    for (int row = 0; row < (int)all.size(); row++)
    {
        const Season& s = all[row];

        table->setItem(row, 0, makeCell(toScreen(s.startDate)));
        table->setItem(row, 1, makeCell(toScreen(s.endDate)));
        table->setItem(row, 2, makeCell(QString::number(s.discountPercent, 'f', 2)));

        bool expired = s.endDate < today;

        QPushButton* statusButton = new QPushButton(
            expired ? "Expired" : (s.isActive ? "Active" : "Inactive"), table);

        if (expired)
        {
            // no click: an expired season can never be used again
            statusButton->setEnabled(false);
            statusButton->setStyleSheet(
                "QPushButton { background-color: #E2E6EF; color: #7A8294; font-weight: bold;"
                " border: none; border-radius: 8px; margin: 5px 14px; }");
        }
        else
        {
            statusButton->setCursor(Qt::PointingHandCursor);
            statusButton->setStyleSheet(s.isActive
                ? "QPushButton { background-color: #15952C; color: white; font-weight: bold; border: none; border-radius: 8px; margin: 5px 14px; }"
                  "QPushButton:hover { background-color: #0F7A22; }"
                : "QPushButton { background-color: #B0B6C3; color: white; font-weight: bold; border: none; border-radius: 8px; margin: 5px 14px; }"
                  "QPushButton:hover { background-color: #98A0B0; }");

            bool makeActive = !s.isActive;

            connect(statusButton, &QPushButton::clicked, this, [this, row, makeActive]()
            {
                onStatusClicked(row, makeActive);
            });
        }

        table->setCellWidget(row, 3, statusButton);
    }
}

void SeasonWindow::onAdd()
{
    double percent = 0;

    if (!readPercent(percent))
    {
        QMessageBox::warning(this, "Discount missing", "Please type the discount in percent.");
        return;
    }

    // 0 can never be saved, a discount must be more than 0
    if (percent <= 0)
    {
        QMessageBox::warning(this, "Discount not allowed", "The discount must be more than 0%.");
        return;
    }

    // too low or too high: ask first, the manager can still go ahead
    QString problem = percentProblem(percent);
    if (!problem.isEmpty())
    {
        QMessageBox::StandardButton answer = QMessageBox::question(
            this, "Are you sure?",
            problem + "\n\nDo you still want to add this season?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (answer != QMessageBox::Yes)
        {
            return;
        }
    }

    Season s;
    s.startDate = startEdit->date().toString(FILE_DATE).toStdString();
    s.endDate = endEdit->date().toString(FILE_DATE).toStdString();
    s.discountPercent = percent;
    s.isActive = true;

    // ask first if another active season covers some of these dates
    seasons.load();
    QString overlap = overlapText(seasons.getAll(), s.startDate, s.endDate, -1);
    if (!overlap.isEmpty())
    {
        QMessageBox::StandardButton answer = QMessageBox::question(
            this, "Are you sure?",
            "These dates overlap an active season:\n\n" + overlap +
            "\nWhen seasons overlap, only the highest discount is used.\n"
            "Do you still want to add this season?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (answer != QMessageBox::Yes)
        {
            return;
        }
    }

    string error;
    if (!seasons.add(s, error))
    {
        QMessageBox::warning(this, "Cannot add season", QString::fromStdString(error));
        return;
    }

    percentEdit->clear();
    refresh();
}

// The row number is the same index that setActive expects
void SeasonWindow::onStatusClicked(int row, bool makeActive)
{
    // switching a season on: ask first if it overlaps another active season
    if (makeActive)
    {
        seasons.load();
        vector<Season> all = seasons.getAll();

        if (row >= 0 && row < (int)all.size())
        {
            QString overlap = overlapText(all, all[row].startDate, all[row].endDate, row);
            if (!overlap.isEmpty())
            {
                QMessageBox::StandardButton answer = QMessageBox::question(
                    this, "Are you sure?",
                    "This season overlaps another active season:\n\n" + overlap +
                    "\nWhen seasons overlap, only the highest discount is used.\n"
                    "Do you still want to switch it on?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

                if (answer != QMessageBox::Yes)
                {
                    return;
                }
            }
        }
    }

    if (!seasons.setActive(row, makeActive))
    {
        QMessageBox::warning(this, "Seasons", "Could not change this season.");
        return;
    }

    refresh();
}
