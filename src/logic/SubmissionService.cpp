#include "SubmissionService.h"

#include <chrono>
#include <filesystem>

#include "Errors.h"
#include "Validation.h"

SubmissionService::SubmissionService(IAssignmentRepository& assignments,
                                     ICourseRepository& courses,
                                     ISubmissionRepository& submissions, IClock& clock)
    : assignments(assignments), courses(courses), submissions(submissions), clock(clock) {}

int SubmissionService::daysLate(Timestamp deadline, Timestamp at) {
    if (at <= deadline) return 0;
    long long secondsLate = std::chrono::duration_cast<std::chrono::seconds>(at - deadline).count();
    const long long secondsPerDay = 24 * 60 * 60;
    return static_cast<int>((secondsLate + secondsPerDay - 1) / secondsPerDay);  // ceiling
}

// The assignment deadline, or the student's own extension if there is one (FR06).
Timestamp SubmissionService::effectiveDeadline(const Assignment& a, int studentId) {
    std::optional<Extension> ext = assignments.findExtension(a.getId(), studentId);
    return a.effectiveDeadlineFor(ext ? &(*ext) : nullptr);
}

Submission SubmissionService::submit(Session& s, int assignmentId, const std::string& filePath) {
    s.require(Action::SUBMIT_ASSIGNMENT);
    int studentId = s.user().getId();

    std::optional<Assignment> a = assignments.findById(assignmentId);
    if (!a) throw NotFoundError("No assignment with that id.");
    if (!courses.isEnrolled(studentId, a->getCourseId())) {
        throw AccessDeniedError("You are not enrolled on this course.");
    }
    requireNotBlank(filePath, "File path");
    if (!std::filesystem::exists(filePath)) {
        throw ValidationError("File not found: " + filePath);
    }

    Timestamp t = clock.now();
    Timestamp deadline = effectiveDeadline(*a, studentId);
    std::optional<Submission> existing = submissions.find(assignmentId, studentId);

    if (!existing && t <= deadline) {  // on time
        Submission sub(assignmentId, studentId, filePath, t, SubmissionStatus::SUBMITTED, 0);
        int id = submissions.insert(sub);
        return *submissions.findById(id);
    }
    if (!existing && t > deadline) {  // late work is always accepted (C2)
        Submission sub(assignmentId, studentId, filePath, t, SubmissionStatus::LATE,
                       daysLate(deadline, t));
        int id = submissions.insert(sub);
        return *submissions.findById(id);
    }
    if (existing->getStatus() == SubmissionStatus::SUBMITTED && t <= deadline) {
        existing->resubmit(filePath, t);  // resubmission (FR17)
        submissions.update(*existing);
        return *existing;
    }
    // Late work, or work that is already marked, cannot be resubmitted (O2).
    throw ValidationError("Resubmission is not allowed.");
}

std::vector<StudentResult> SubmissionService::resultsFor(Session& s) {
    s.require(Action::VIEW_RESULTS);
    return submissions.resultsForStudent(s.user().getId());
}
