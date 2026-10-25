#include "ModerationService.h"

#include "Errors.h"
#include "Validation.h"

ModerationService::ModerationService(ISubmissionRepository& submissions,
                                     IAssignmentRepository& assignments,
                                     ICourseRepository& courses, IAuditRepository& audit,
                                     IUnitOfWork& tx, IClock& clock)
    : submissions(submissions),
      assignments(assignments),
      courses(courses),
      audit(audit),
      tx(tx),
      clock(clock) {}

int ModerationService::hodDepartment(Session& s) {
    std::optional<int> dept = courses.departmentOfHod(s.user().getId());
    if (!dept) throw AccessDeniedError("You are not the head of any department.");
    return *dept;
}

Assignment ModerationService::assignmentInDepartment(Session& s, int assignmentId) {
    std::optional<Assignment> a = assignments.findById(assignmentId);
    if (!a) throw NotFoundError("No assignment with that id.");
    std::optional<Course> course = courses.findCourse(a->getCourseId());
    if (!course || course->getDepartmentId() != hodDepartment(s)) {
        throw AccessDeniedError("This assignment is not in your department.");
    }
    return *a;
}

Submission ModerationService::findSubmission(int submissionId) {
    std::optional<Submission> sub = submissions.findById(submissionId);
    if (!sub) throw NotFoundError("No submission with that id.");
    return *sub;
}

// FR10: marked work in the HoD's department that is waiting for moderation.
std::vector<Submission> ModerationService::pendingForDepartment(Session& s) {
    s.require(Action::REVIEW_MARKS);
    return submissions.findByDepartmentAndStatus(hodDepartment(s), SubmissionStatus::MARKED);
}

// The mark is accepted as it is, so there is no mark change to audit.
void ModerationService::approve(Session& s, int submissionId) {
    s.require(Action::MODERATE_MARK);
    Submission sub = findSubmission(submissionId);
    assignmentInDepartment(s, sub.getAssignmentId());
    sub.approve(s.user().getId(), clock.now());  // MARKED -> MODERATED
    submissions.update(sub);
}

void ModerationService::adjust(Session& s, int submissionId, double newMark,
                               const std::string& reason) {
    s.require(Action::MODERATE_MARK);
    Submission sub = findSubmission(submissionId);
    Assignment a = assignmentInDepartment(s, sub.getAssignmentId());
    requireNotBlank(reason, "Reason");  // FR11: a reason is required
    if (newMark < 0 || newMark > a.getMaxMark()) {
        throw ValidationError("Mark must be between 0 and " + std::to_string(a.getMaxMark()) +
                              ".");
    }

    std::optional<double> oldMark = sub.getMark();
    Timestamp now = clock.now();
    int hodId = s.user().getId();

    tx.runInTransaction([&] {
        sub.adjustMark(newMark, hodId, now);  // MARKED/MODERATED -> MODERATED
        submissions.update(sub);
        audit.append(
            AuditEntry(0, sub.getId(), hodId, now, AuditAction::MODERATE, oldMark, newMark, reason));
    });
}

// FR12: releases every MODERATED submission of the assignment.
// Submissions that are still MARKED are not released and stay hidden (FR13).
int ModerationService::release(Session& s, int assignmentId) {
    s.require(Action::RELEASE_MARKS);
    assignmentInDepartment(s, assignmentId);
    Timestamp t = clock.now();
    std::vector<Submission> moderated =
        submissions.findByAssignmentAndStatus(assignmentId, SubmissionStatus::MODERATED);

    tx.runInTransaction([&] {
        for (Submission& sub : moderated) {
            sub.release(t);  // MODERATED -> RELEASED
            submissions.update(sub);
        }
    });
    return static_cast<int>(moderated.size());
}
