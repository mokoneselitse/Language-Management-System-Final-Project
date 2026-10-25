#include "AuditService.h"

AuditService::AuditService(IAuditRepository& audit) : audit(audit) {}

std::vector<AuditEntry> AuditService::history(Session& s, int submissionId) {
    s.require(Action::VIEW_AUDIT_LOG);
    return audit.forSubmission(submissionId);
}
