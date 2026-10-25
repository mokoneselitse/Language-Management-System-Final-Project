// AuditEntry.h - one row of the mark-change audit log (who, when, old, new, why).
#pragma once

#include <optional>
#include <string>
#include <utility>

#include "Types.h"

class AuditEntry {
public:
    AuditEntry(int id, int submissionId, int changedBy, Timestamp changedAt, AuditAction action,
               std::optional<double> oldMark, double newMark, std::string reason)
        : id(id),
          submissionId(submissionId),
          changedBy(changedBy),
          changedAt(changedAt),
          action(action),
          oldMark(oldMark),
          newMark(newMark),
          reason(std::move(reason)) {}

    int getId() const { return id; }
    int getSubmissionId() const { return submissionId; }
    int getChangedBy() const { return changedBy; }
    Timestamp getChangedAt() const { return changedAt; }
    AuditAction getAction() const { return action; }
    std::optional<double> getOldMark() const { return oldMark; }
    double getNewMark() const { return newMark; }
    std::string getReason() const { return reason; }

private:
    int id;
    int submissionId;
    int changedBy;
    Timestamp changedAt;
    AuditAction action;
    std::optional<double> oldMark;
    double newMark;
    std::string reason;
};
