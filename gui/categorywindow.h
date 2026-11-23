#ifndef CATEGORYWINDOW_H
#define CATEGORYWINDOW_H
#include <string>
#include <QDialog>
#include "data/ICategoryRepository.h"

class QTableWidget;

// Manager screen: lists categories, lets the manager add and edit them.
// It only talks to ICategoryRepository, it never opens a file.
class CategoryWindow : public QDialog
{
private:
    ICategoryRepository* repository;
    QTableWidget* table;

    // Reloads the table from the repository
    void refresh();

    // Shows the input form. Returns true if the user pressed OK.
    // When editing, the id is read-only.
    bool askCategory(Category& category, bool editing);

    void onAdd();
    void onEdit();

public:
    explicit CategoryWindow(ICategoryRepository* repository, QWidget* parent = nullptr);
};

#endif