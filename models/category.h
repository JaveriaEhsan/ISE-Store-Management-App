#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>

struct Category
{
    std::string categoryId;    // 2 digits, first two digits of the barcode
    std::string name;
    std::string acronym;
};

#endif