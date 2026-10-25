// SubmissionService.h - submitting work and viewing results (FR16, FR17, FR19).
// Follows the "Submit assignment" sequence diagram.
#pragma once

#include <string>
#include <vector>

#include "IClock.h"
#include "Repositories.h"
#include "Session.h"

class SubmissionService {
public:
    SubmissionService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IClock& clock);

    Submission submit(Session& s, int assignmentId, const std::string& filePath);
    std::vector<StudentResult> resultsFor(Session& s);

    // Whole days late, rounded up: 1 second late counts as 1 day.
    static int daysLate(Timestamp deadline, Timestamp at);

private:
    Timestamp effectiveDeadline(const Assignment& a, int studentId);

    IAssignmentRepository& assignments;
    ICourseRepository& courses;
    ISubmissionRepository& submissions;
    IClock& clock;
};
