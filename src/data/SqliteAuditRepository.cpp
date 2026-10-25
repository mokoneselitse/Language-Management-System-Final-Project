#include "SqliteRepositories.h"

#include "SqlHelpers.h"

// audit_log is append-only: there is no update or delete here, and the
// database triggers reject them anyway (FR22).
void SqliteAuditRepository::append(const AuditEntry& e) {
    Statement st = db.prepare(
        "INSERT INTO audit_log (submission_id, changed_by, changed_at, action, old_mark, "
        "new_mark, reason) VALUES (?, ?, ?, ?, ?, ?, ?);");
    st.bind(1, e.getSubmissionId())
        .bind(2, e.getChangedBy())
        .bind(3, toUnixSeconds(e.getChangedAt()))
        .bind(4, toString(e.getAction()));
    bindOptional(st, 5, e.getOldMark());
    st.bind(6, e.getNewMark()).bind(7, e.getReason());
    st.step();
}

std::vector<AuditEntry> SqliteAuditRepository::forSubmission(int submissionId) {
    Statement st = db.prepare(
        "SELECT audit_id, submission_id, changed_by, changed_at, action, old_mark, new_mark, "
        "reason FROM audit_log WHERE submission_id = ? ORDER BY changed_at, audit_id;");
    st.bind(1, submissionId);
    std::vector<AuditEntry> list;
    while (st.step()) {
        list.emplace_back(st.columnInt(0), st.columnInt(1), st.columnInt(2), readTime(st, 3),
                          auditActionFromString(st.columnText(4)), readOptionalDouble(st, 5),
                          st.columnDouble(6), st.columnText(7));
    }
    return list;
}
