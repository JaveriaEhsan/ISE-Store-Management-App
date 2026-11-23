#include "gui/categorywindow.h"
#include <string>
#include <vector>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
using namespace std;

// The file stores _ instead of spaces, the screen shows spaces
static string withSpaces(string text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == '_')
        {
            text[i] = ' ';
        }
    }
    return text;
}

CategoryWindow::CategoryWindow(ICategoryRepository* repository, QWidget* parent)
    : QDialog(parent), repository(repository)
{
    setWindowTitle("Categories");
    resize(480, 360);

    // Table with three columns
    table = new QTableWidget(0, 3, this);
    table->setHorizontalHeaderLabels({ "Id", "Name", "Acronym" });
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);

    QPushButton* addButton = new QPushButton("Add", this);
    QPushButton* editButton = new QPushButton("Edit", this);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(addButton);
    buttons->addWidget(editButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addLayout(buttons);

    connect(addButton, &QPushButton::clicked, this, [this]() { onAdd(); });
    connect(editButton, &QPushButton::clicked, this, [this]() { onEdit(); });
    // Double click on a row also opens the edit form
    connect(table, &QTableWidget::cellDoubleClicked, this, [this]() { onEdit(); });

    refresh();
}

void CategoryWindow::refresh()
{
    vector<Category> all = repository->getAll();
    table->setRowCount(0);

    for (size_t i = 0; i < all.size(); i++)
    {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(all[i].categoryId)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(withSpaces(all[i].name))));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(withSpaces(all[i].acronym))));
    }
}

bool CategoryWindow::askCategory(Category& category, bool editing)
{
    QDialog form(this);
    form.setWindowTitle(editing ? "Edit category" : "Add category");

    QLineEdit* idEdit = new QLineEdit(QString::fromStdString(category.categoryId), &form);
    QLineEdit* nameEdit = new QLineEdit(QString::fromStdString(withSpaces(category.name)), &form);
    QLineEdit* acronymEdit = new QLineEdit(QString::fromStdString(withSpaces(category.acronym)), &form);

    idEdit->setMaxLength(2);
    idEdit->setPlaceholderText("2 digits, for example 10");
    idEdit->setReadOnly(editing);   // the id never changes

    QFormLayout* fields = new QFormLayout();
    fields->addRow("Id:", idEdit);
    fields->addRow("Name:", nameEdit);
    fields->addRow("Acronym:", acronymEdit);

    QDialogButtonBox* box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &form);
    connect(box, &QDialogButtonBox::accepted, &form, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &form, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(&form);
    layout->addLayout(fields);
    layout->addWidget(box);

    if (form.exec() != QDialog::Accepted)
    {
        return false;
    }

    category.categoryId = idEdit->text().trimmed().toStdString();
    category.name = nameEdit->text().trimmed().toStdString();
    category.acronym = acronymEdit->text().trimmed().toStdString();
    return true;
}

void CategoryWindow::onAdd()
{
    Category category;
    if (!askCategory(category, false))
    {
        return;
    }

    string error;
    if (!repository->add(category, error))
    {
        QMessageBox::warning(this, "Cannot add category", QString::fromStdString(error));
        return;
    }
    refresh();
}

void CategoryWindow::onEdit()
{
    int row = table->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Edit category", "Please select a category first.");
        return;
    }

    string id = table->item(row, 0)->text().toStdString();
    Category category;
    if (!repository->findById(id, category))
    {
        QMessageBox::warning(this, "Edit category", "This category no longer exists.");
        refresh();
        return;
    }

    if (!askCategory(category, true))
    {
        return;
    }

    string error;
    if (!repository->update(category, error))
    {
        QMessageBox::warning(this, "Cannot update category", QString::fromStdString(error));
        return;
    }
    refresh();
}