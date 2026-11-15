#ifndef BILLREPOSITORY_H   
#define BILLREPOSITORY_H

#include "../models/bill.h"   // gives the Bill struct

class BillRepository
{
public:
    bool save(const Bill& bill);   // writes one line to file
};

#endif