#include "MarkingService.h"

#include "Errors.h"
#include "Validation.h"

MarkingService::MarkingService(ISubmissionRepository& submissions,
                               IAssignmentRepository& assignments, IAuditRepository& audit,
                               ILatePenaltyPolicy& penalty, IUnitOfWork& tx, IClock& clock)
    : submissions(submissions),
      assignments(assignments),
      audit(audit),
      penalty(penalty),
      tx(tx),
      clock(clock) {}

Submission MarkingService::findSubmission(int submissionId) {
    std::optional<Submission> sub = submissions.findById(submissionId);
    if (!sub) throw NotFoundError("No submission with that id.");
    return *sub;
}

Assignment MarkingService::ownAssignment(Session& s, const Submission& sub) {
    std::optional<Assignment> a = assignments.findById(sub.getAssignmentId());
    if (!a) throw NotFoundError("Assignment not found.");
    // Only the lecturer who set the assignment may mark it.
    if (a->getCreatedBy() != s.user().getId()) {
        throw AccessDeniedError("This submission is not for one of your assignments.");
    }
    return *a;
}

static void checkRawMark(double rawMark, double maxMark) {
    if (rawMark < 0 || rawMark > maxMark) {  // FR07
        throw ValidationError("Mark must be between 0 and " + std::to_string(maxMark) + ".");
    }
}

void MarkingService::mark(Session& s, int submissionId, double rawMark,
                          const std::string& feedback) {
    s.require(Action::MARK_SUBMISSION);
    Submission sub = findSubmission(submissionId);
    Assignment a = ownAssignment(s, sub);
    if (sub.getStatus() != SubmissionStatus::SUBMITTED &&
        sub.getStatus() != SubmissionStatus::LATE) {
        throw InvalidTransitionError("This submission is already marked. Use re-mark instead.");
    }
    checkRawMark(rawMark, a.getMaxMark());
    requireNotBlank(feedback, "Feedback");

    double finalMark = penalty.apply(rawMark, a.getMaxMark(), sub.getDaysLate());  // FR18
    Timestamp now = clock.now();
    int lecturerId = s.user().getId();

    tx.runInTransaction([&] {
        sub.recordMark(rawMark, finalMark, feedback, lecturerId, now);
        submissions.update(sub);
        audit.append(AuditEntry(0, sub.getId(), lecturerId, now, AuditAction::MARK,
                                std::nullopt, finalMark, "Initial mark"));
    });
}

void MarkingService::remark(Session& s, int submissionId, double rawMark,
                            const std::string& reason) {
    s.require(Action::MARK_SUBMISSION);
    Submission sub = findSubmission(submissionId);
    Assignment a = ownAssignment(s, sub);
    if (sub.getStatus() != SubmissionStatus::MARKED) {
        throw InvalidTransitionError("Only a MARKED submission can be re-marked.");
    }
    checkRawMark(rawMark, a.getMaxMark());
    requireNotBlank(reason, "Reason");

    std::optional<double> oldMark = sub.getMark();
    double finalMark = penalty.apply(rawMark, a.getMaxMark(), sub.getDaysLate());
    Timestamp now = clock.now();
    int lecturerId = s.user().getId();

    tx.runInTransaction([&] {
        sub.recordMark(rawMark, finalMark, sub.getFeedback(), lecturerId, now);
        submissions.update(sub);
        audit.append(AuditEntry(0, sub.getId(), lecturerId, now, AuditAction::REMARK, oldMark,
                                finalMark, reason));
    });
}
