#ifndef IDGENERATOR_H   
#define IDGENERATOR_H

#include <string>

class IdGenerator
{
public:
    // 8 digits: 6-digit serial + 2-digit day, like "00000216"
    std::string nextOrderId();

    // 9 digits: running serial, like "000000002"
    std::string nextBillId();
};

#endif