#ifndef APPROVAL_H
#define APPROVAL_H

#include <string>

// The four states of an approval request
const std::string APPROVAL_PENDING  = "pending";
const std::string APPROVAL_APPROVED = "approved";
const std::string APPROVAL_REJECTED = "rejected";
const std::string APPROVAL_USED     = "used";

// One row of Approvals.txt
struct Approval
{
    std::string requestId;       // 8 digits
    std::string orderId;         // the order id from Order.txt
    std::string requestDate;     // 2026-11-23
    std::string requestTime;     // 12:40
    std::string status;          // one of the APPROVAL_ values
    std::string decisionDate;    // - until the manager decides
    std::string decisionTime;    // - until the manager decides
};

#endif