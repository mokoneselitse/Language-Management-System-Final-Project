#include "Submission.h"

#include <utility>

#include "Errors.h"

Submission::Submission(int assignmentId, int studentId, std::string filePath,
                       Timestamp submittedAt, SubmissionStatus status, int daysLate)
    : assignmentId(assignmentId),
      studentId(studentId),
      filePath(std::move(filePath)),
      submittedAt(submittedAt),
      status(status),
      daysLate(daysLate) {
    // The initial state can only be SUBMITTED (on time) or LATE.
    if (status != SubmissionStatus::SUBMITTED && status != SubmissionStatus::LATE) {
        throw InvalidTransitionError("A new submission must start as SUBMITTED or LATE.");
    }
}

Submission::Submission(int id, int assignmentId, int studentId, std::string filePath,
                       Timestamp submittedAt, int attemptNo, SubmissionStatus status, int daysLate,
                       std::optional<double> rawMark, std::optional<double> mark,
                       std::string feedback, std::optional<int> markedBy,
                       std::optional<Timestamp> markedAt, std::optional<int> moderatedBy,
                       std::optional<Timestamp> moderatedAt, std::optional<Timestamp> releasedAt)
    : id(id),
      assignmentId(assignmentId),
      studentId(studentId),
      filePath(std::move(filePath)),
      submittedAt(submittedAt),
      attemptNo(attemptNo),
      status(status),
      daysLate(daysLate),
      rawMark(rawMark),
      mark(mark),
      feedback(std::move(feedback)),
      markedBy(markedBy),
      markedAt(markedAt),
      moderatedBy(moderatedBy),
      moderatedAt(moderatedAt),
      releasedAt(releasedAt) {}

// The allowed transitions, taken from the submission state transition table.
void Submission::transitionTo(SubmissionStatus next) {
    bool allowed = false;
    switch (status) {
        case SubmissionStatus::SUBMITTED:
            allowed = (next == SubmissionStatus::SUBMITTED || next == SubmissionStatus::MARKED);
            break;
        case SubmissionStatus::LATE:
            allowed = (next == SubmissionStatus::MARKED);
            break;
        case SubmissionStatus::MARKED:
            allowed = (next == SubmissionStatus::MARKED || next == SubmissionStatus::MODERATED);
            break;
        case SubmissionStatus::MODERATED:
            allowed = (next == SubmissionStatus::MODERATED || next == SubmissionStatus::RELEASED);
            break;
        case SubmissionStatus::RELEASED:
            allowed = false;  // final state
            break;
    }
    if (!allowed) {
        throw InvalidTransitionError("Cannot go from " + toString(status) + " to " +
                                     toString(next) + ".");
    }
    status = next;
}

void Submission::resubmit(const std::string& path, Timestamp at) {
    transitionTo(SubmissionStatus::SUBMITTED);  // only allowed from SUBMITTED
    filePath = path;
    submittedAt = at;
    attemptNo++;
}

void Submission::recordMark(double raw, double finalMark, const std::string& feedbackText,
                            int by, Timestamp at) {
    transitionTo(SubmissionStatus::MARKED);
    rawMark = raw;
    mark = finalMark;
    feedback = feedbackText;
    markedBy = by;
    markedAt = at;
}

void Submission::approve(int by, Timestamp at) {
    if (status != SubmissionStatus::MARKED) {
        throw InvalidTransitionError("Only a MARKED submission can be approved.");
    }
    transitionTo(SubmissionStatus::MODERATED);
    moderatedBy = by;
    moderatedAt = at;
}

void Submission::adjustMark(double newMark, int by, Timestamp at) {
    transitionTo(SubmissionStatus::MODERATED);  // from MARKED or MODERATED
    mark = newMark;
    moderatedBy = by;
    moderatedAt = at;
}

void Submission::release(Timestamp at) {
    transitionTo(SubmissionStatus::RELEASED);  // only from MODERATED
    releasedAt = at;
}

bool Submission::isFeedbackVisibleToStudent() const {
    return status == SubmissionStatus::MARKED || status == SubmissionStatus::MODERATED ||
           status == SubmissionStatus::RELEASED;
}

bool Submission::isMarkVisibleToStudent() const { return status == SubmissionStatus::RELEASED; }
