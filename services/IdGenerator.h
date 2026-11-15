#ifndef IDGENERATOR_H   
#define IDGENERATOR_H

#include <string>
using namespace std;

class IdGenerator
{
public:
    // 8 digits: 6-digit serial + 2-digit day, like "00000216"
    string nextOrderId();

    // 9 digits: running serial, like "000000002"
    string nextBillId();
};

#endif