#include "Types.h"

#include "Errors.h"

std::string toString(Role role) {
    switch (role) {
        case Role::STUDENT: return "STUDENT";
        case Role::LECTURER: return "LECTURER";
        case Role::HOD: return "HOD";
        case Role::ADMIN: return "ADMIN";
    }
    return "UNKNOWN";
}

std::string toString(SubmissionStatus status) {
    switch (status) {
        case SubmissionStatus::SUBMITTED: return "SUBMITTED";
        case SubmissionStatus::LATE: return "LATE";
        case SubmissionStatus::MARKED: return "MARKED";
        case SubmissionStatus::MODERATED: return "MODERATED";
        case SubmissionStatus::RELEASED: return "RELEASED";
    }
    return "UNKNOWN";
}

std::string toString(AuditAction action) {
    switch (action) {
        case AuditAction::MARK: return "MARK";
        case AuditAction::REMARK: return "REMARK";
        case AuditAction::MODERATE: return "MODERATE";
    }
    return "UNKNOWN";
}

Role roleFromString(const std::string& text) {
    if (text == "STUDENT") return Role::STUDENT;
    if (text == "LECTURER") return Role::LECTURER;
    if (text == "HOD") return Role::HOD;
    if (text == "ADMIN") return Role::ADMIN;
    throw ValidationError("Unknown role: " + text);
}

SubmissionStatus statusFromString(const std::string& text) {
    if (text == "SUBMITTED") return SubmissionStatus::SUBMITTED;
    if (text == "LATE") return SubmissionStatus::LATE;
    if (text == "MARKED") return SubmissionStatus::MARKED;
    if (text == "MODERATED") return SubmissionStatus::MODERATED;
    if (text == "RELEASED") return SubmissionStatus::RELEASED;
    throw ValidationError("Unknown submission status: " + text);
}

AuditAction auditActionFromString(const std::string& text) {
    if (text == "MARK") return AuditAction::MARK;
    if (text == "REMARK") return AuditAction::REMARK;
    if (text == "MODERATE") return AuditAction::MODERATE;
    throw ValidationError("Unknown audit action: " + text);
}

long long toUnixSeconds(Timestamp t) {
    return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
}

Timestamp fromUnixSeconds(long long seconds) {
    return Timestamp(std::chrono::seconds(seconds));
}
