// ModerationService.h - HoD review, moderation and release (FR10 - FR12, FR22).
// Follows the "Moderate and release marks" sequence diagram.
#pragma once

#include <string>
#include <vector>

#include "IClock.h"
#include "IUnitOfWork.h"
#include "Repositories.h"
#include "Session.h"

class ModerationService {
public:
    ModerationService(ISubmissionRepository& submissions, IAssignmentRepository& assignments,
                      ICourseRepository& courses, IAuditRepository& audit, IUnitOfWork& tx,
                      IClock& clock);

    std::vector<Submission> pendingForDepartment(Session& s);
    void approve(Session& s, int submissionId);
    void adjust(Session& s, int submissionId, double newMark, const std::string& reason);
    int release(Session& s, int assignmentId);  // returns how many marks were released

private:
    int hodDepartment(Session& s);
    // The assignment, if its course is in the HoD's department (else AccessDeniedError).
    Assignment assignmentInDepartment(Session& s, int assignmentId);
    Submission findSubmission(int submissionId);

    ISubmissionRepository& submissions;
    IAssignmentRepository& assignments;
    ICourseRepository& courses;
    IAuditRepository& audit;
    IUnitOfWork& tx;
    IClock& clock;
};
