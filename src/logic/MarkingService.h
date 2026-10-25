// MarkingService.h - marking and re-marking (FR07, FR08, FR18, FR22).
// The late penalty comes from the ILatePenaltyPolicy strategy, and every mark
// change is saved together with its audit entry in one transaction.
#pragma once

#include <string>
#include <vector>

#include "IClock.h"
#include "IUnitOfWork.h"
#include "LatePenalty.h"
#include "Repositories.h"
#include "Session.h"

class MarkingService {
public:
    MarkingService(ISubmissionRepository& submissions, IAssignmentRepository& assignments,
                   IAuditRepository& audit, ILatePenaltyPolicy& penalty, IUnitOfWork& tx,
                   IClock& clock);

    void mark(Session& s, int submissionId, double rawMark, const std::string& feedback);
    void remark(Session& s, int submissionId, double rawMark, const std::string& reason);

    // Added during coding: lets the lecturer menu list the submissions of one
    // of their assignments (SUBMITTED, LATE and MARKED) so they can pick one.
    std::vector<Submission> submissionsFor(Session& s, int assignmentId);

private:
    Submission findSubmission(int submissionId);
    // The submission's assignment; throws if the lecturer did not set it.
    Assignment ownAssignment(Session& s, const Submission& sub);

    ISubmissionRepository& submissions;
    IAssignmentRepository& assignments;
    IAuditRepository& audit;
    ILatePenaltyPolicy& penalty;
    IUnitOfWork& tx;
    IClock& clock;
};
