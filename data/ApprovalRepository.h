#ifndef APPROVALREPOSITORY_H
#define APPROVALREPOSITORY_H

#include <string>
#include <vector>
#include "../models/approval.h"

// Reads and writes Approvals.txt (one line per request, 7 fields, # at the end).
// It only stores and loads. The rules (7 days, no duplicate requests) live in OrderService.
class ApprovalRepository
{
public:
    std::vector<Approval> getAll() const;
    std::vector<Approval> findByOrderId(const std::string& orderId) const;
    std::vector<Approval> findByStatus(const std::string& status) const;   // e.g. all pending

    // The newest request of this order that is pending or approved. False if there is none.
    bool findActiveByOrderId(const std::string& orderId, Approval& result) const;

    // Next free 8-digit request id
    std::string nextId() const;

    // Appends a request. The caller sets all fields (status normally "pending").
    bool add(const Approval& approval, std::string& error);

    // Changes the status of one request and writes the decision date/time when needed.
    bool setStatus(const std::string& requestId, const std::string& status, std::string& error);
};

#endif