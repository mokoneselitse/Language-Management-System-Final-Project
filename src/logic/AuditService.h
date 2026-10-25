// AuditService.h - viewing the change history of a mark (FR22).
#pragma once

#include <vector>

#include "Repositories.h"
#include "Session.h"

class AuditService {
public:
    explicit AuditService(IAuditRepository& audit);
    std::vector<AuditEntry> history(Session& s, int submissionId);

private:
    IAuditRepository& audit;
};
