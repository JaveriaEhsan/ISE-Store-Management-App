#include "ApprovalRepository.h"
#include "TextFormat.h"
#include "../services/DateUtil.h"
#include "../config.h"

#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// Turns one file line (7 fields) into an Approval. Returns false if the line is bad.
static bool parseApproval(const string& line, Approval& a)
{
    vector<string> f = splitLine(line);

    if (f.size() != 7)
    {
        return false;
    }

    a.requestId = f[0];
    a.orderId = f[1];
    a.requestDate = f[2];
    a.requestTime = f[3];
    a.status = f[4];
    a.decisionDate = f[5];
    a.decisionTime = f[6];
    return true;
}

// The only place the line format is written
static string makeLine(const Approval& a)
{
    ostringstream out;

    out << a.requestId << " "
        << a.orderId << " "
        << a.requestDate << " "
        << a.requestTime << " "
        << a.status << " "
        << fixText(a.decisionDate) << " "     // empty is written as -
        << fixText(a.decisionTime)
        << "#";

    return out.str();
}

static bool validStatus(const string& status)
{
    return status == APPROVAL_PENDING || status == APPROVAL_APPROVED
        || status == APPROVAL_REJECTED || status == APPROVAL_USED;
}

// True if the text is empty or has a space (it would shift the columns)
static bool badField(const string& text)
{
    return text.empty() || text.find(' ') != string::npos;
}

// Returns "" if fine, otherwise the reason
static string checkApproval(const Approval& a)
{
    if (badField(a.requestId) || badField(a.orderId))
    {
        return "The request id and order id must not be empty or contain spaces.";
    }
    if (dateKey(a.requestDate).empty() || badField(a.requestTime))
    {
        return "The request date or time is not valid.";
    }
    if (!validStatus(a.status))
    {
        return "The approval status is not allowed.";
    }
    return "";
}

// Rewrites the whole file from a list of lines
static bool writeLines(const vector<string>& lines)
{
    ofstream out(APPROVAL_FILE, ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    for (const string& l : lines)
    {
        out << l << "\n";
    }
    return out.good();
}

vector<Approval> ApprovalRepository::getAll() const
{
    vector<Approval> result;
    ifstream file(APPROVAL_FILE);      // a missing file means no requests
    string line;

    while (getline(file, line))
    {
        Approval a;
        if (parseApproval(line, a))
        {
            result.push_back(a);
        }
    }
    return result;
}

vector<Approval> ApprovalRepository::findByOrderId(const string& orderId) const
{
    vector<Approval> result;
    vector<Approval> all = getAll();

    for (const Approval& a : all)
    {
        if (a.orderId == orderId)
        {
            result.push_back(a);
        }
    }
    return result;
}

vector<Approval> ApprovalRepository::findByStatus(const string& status) const
{
    vector<Approval> result;
    vector<Approval> all = getAll();

    for (const Approval& a : all)
    {
        if (a.status == status)
        {
            result.push_back(a);
        }
    }
    return result;
}

bool ApprovalRepository::findActiveByOrderId(const string& orderId, Approval& result) const
{
    vector<Approval> list = findByOrderId(orderId);

    // the file is in saving order, so look from the end for the newest
    for (size_t i = list.size(); i > 0; i--)
    {
        const Approval& a = list[i - 1];
        if (a.status == APPROVAL_PENDING || a.status == APPROVAL_APPROVED)
        {
            result = a;
            return true;
        }
    }
    return false;
}

string ApprovalRepository::nextId() const
{
    long long biggest = 0;
    vector<Approval> all = getAll();

    for (const Approval& a : all)
    {
        try
        {
            long long id = stoll(a.requestId);
            if (id > biggest) biggest = id;
        }
        catch (...) {}
    }

    string id = to_string(biggest + 1);
    while (id.size() < 8) id = "0" + id;
    return id;
}

bool ApprovalRepository::add(const Approval& approval, string& error)
{
    error = checkApproval(approval);
    if (!error.empty())
    {
        return false;
    }

    // two requests must never share an id
    for (const Approval& a : getAll())
    {
        if (a.requestId == approval.requestId)
        {
            error = "A request with this id already exists.";
            return false;
        }
    }

    ofstream file(APPROVAL_FILE, ios::app);
    if (!file.is_open())
    {
        error = "The approval file could not be opened.";
        return false;
    }

    file << makeLine(approval) << "\n";

    if (!file.good())
    {
        error = "Writing to the approval file failed.";
        return false;
    }
    return true;
}

bool ApprovalRepository::setStatus(const string& requestId, const string& status, string& error)
{
    error.clear();

    if (!validStatus(status))
    {
        error = "The approval status is not allowed.";
        return false;
    }

    vector<string> lines;
    bool found = false;

    {
        ifstream file(APPROVAL_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            Approval a;
            if (parseApproval(line, a) && a.requestId == requestId)
            {
                // allowed steps: pending -> approved/rejected, approved -> used
                bool ok = (a.status == APPROVAL_PENDING &&
                           (status == APPROVAL_APPROVED || status == APPROVAL_REJECTED))
                       || (a.status == APPROVAL_APPROVED && status == APPROVAL_USED);
                if (!ok)
                {
                    error = "This request cannot change from " + a.status + " to " + status + ".";
                    return false;
                }

                a.status = status;
                if (status == APPROVAL_APPROVED || status == APPROVAL_REJECTED)
                {
                    a.decisionDate = todayDate();      // the manager's decision time
                    a.decisionTime = nowTime();
                }
                line = makeLine(a);
                found = true;
            }
            lines.push_back(line);
        }
    }

    if (!found)
    {
        error = "No request with this id.";
        return false;
    }

    if (!writeLines(lines))
    {
        error = "Writing to the approval file failed.";
        return false;
    }
    return true;
}