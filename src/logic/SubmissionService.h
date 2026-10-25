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
    // storageFolder: where copies of submitted files are kept (e.g. "submissions").
    SubmissionService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IClock& clock,
                      std::string storageFolder);

    Submission submit(Session& s, int assignmentId, const std::string& filePath);
    std::vector<StudentResult> resultsFor(Session& s);

    // Whole days late, rounded up: 1 second late counts as 1 day.
    static int daysLate(Timestamp deadline, Timestamp at);

private:
    Timestamp effectiveDeadline(const Assignment& a, int studentId);
    // Copies the student's file into the storage folder and returns the new path,
    // so the submission is kept even if the student later deletes their file.
    std::string storeFile(const std::string& sourcePath, int assignmentId, int studentId,
                          int attemptNo);

    IAssignmentRepository& assignments;
    ICourseRepository& courses;
    ISubmissionRepository& submissions;
    IClock& clock;
    std::string storageFolder;
};
