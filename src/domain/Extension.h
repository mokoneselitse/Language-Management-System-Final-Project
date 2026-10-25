// Extension.h - an individual deadline extension for one student on one assignment.
#pragma once

#include <string>
#include <utility>

#include "Types.h"

class Extension {
public:
    Extension(int id, int assignmentId, int studentId, Timestamp newDeadline,
              std::string reason, int grantedBy, Timestamp grantedAt)
        : id(id),
          assignmentId(assignmentId),
          studentId(studentId),
          newDeadline(newDeadline),
          reason(std::move(reason)),
          grantedBy(grantedBy),
          grantedAt(grantedAt) {}

    int getId() const { return id; }
    int getAssignmentId() const { return assignmentId; }
    int getStudentId() const { return studentId; }
    Timestamp getNewDeadline() const { return newDeadline; }
    std::string getReason() const { return reason; }
    int getGrantedBy() const { return grantedBy; }
    Timestamp getGrantedAt() const { return grantedAt; }

private:
    int id;
    int assignmentId;
    int studentId;
    Timestamp newDeadline;
    std::string reason;
    int grantedBy;
    Timestamp grantedAt;
};
