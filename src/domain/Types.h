// Types.h - enumerations and the Timestamp type from the domain class diagram.
#pragma once

#include <chrono>
#include <string>

// Timestamp = std::chrono::system_clock::time_point (see domain model note)
using Timestamp = std::chrono::system_clock::time_point;

enum class Role { STUDENT, LECTURER, HOD, ADMIN };

enum class Action {
    VIEW_COURSES,
    VIEW_ASSIGNMENTS,
    SUBMIT_ASSIGNMENT,
    VIEW_RESULTS,
    CREATE_ASSIGNMENT,
    GRANT_EXTENSION,
    MARK_SUBMISSION,
    VIEW_OWN_ASSIGNMENTS,
    REVIEW_MARKS,
    MODERATE_MARK,
    RELEASE_MARKS,
    VIEW_AUDIT_LOG,
    MANAGE_USERS,
    MANAGE_COURSES,
    ASSIGN_LECTURER,
    ENROL_STUDENT
};

enum class SubmissionStatus { SUBMITTED, LATE, MARKED, MODERATED, RELEASED };

enum class AuditAction { MARK, REMARK, MODERATE };

// Conversions used when reading and writing the database (TEXT columns).
std::string toString(Role role);
std::string toString(SubmissionStatus status);
std::string toString(AuditAction action);
Role roleFromString(const std::string& text);
SubmissionStatus statusFromString(const std::string& text);
AuditAction auditActionFromString(const std::string& text);

// Timestamps are stored in the database as INTEGER Unix seconds (UTC).
long long toUnixSeconds(Timestamp t);
Timestamp fromUnixSeconds(long long seconds);
