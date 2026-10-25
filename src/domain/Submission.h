// Submission.h - a student's submission and its life cycle
// (see the Submission state machine diagram).
//
// Every status change goes through the private transitionTo(), which throws
// InvalidTransitionError for any move the state machine does not allow.
#pragma once

#include <optional>
#include <string>

#include "Types.h"

class Submission {
public:
    // A brand-new submission (status must be SUBMITTED or LATE, attempt 1).
    Submission(int assignmentId, int studentId, std::string filePath, Timestamp submittedAt,
               SubmissionStatus status, int daysLate);

    // A submission loaded from the database (all columns).
    Submission(int id, int assignmentId, int studentId, std::string filePath,
               Timestamp submittedAt, int attemptNo, SubmissionStatus status, int daysLate,
               std::optional<double> rawMark, std::optional<double> mark, std::string feedback,
               std::optional<int> markedBy, std::optional<Timestamp> markedAt,
               std::optional<int> moderatedBy, std::optional<Timestamp> moderatedAt,
               std::optional<Timestamp> releasedAt);

    // ---- operations from the class diagram
    void resubmit(const std::string& path, Timestamp at);            // SUBMITTED -> SUBMITTED
    void recordMark(double raw, double finalMark, const std::string& feedback, int by,
                    Timestamp at);                                    // SUBMITTED/LATE/MARKED -> MARKED
    void approve(int by, Timestamp at);                               // MARKED -> MODERATED
    void adjustMark(double newMark, int by, Timestamp at);            // MARKED/MODERATED -> MODERATED
    void release(Timestamp at);                                       // MODERATED -> RELEASED
    bool isFeedbackVisibleToStudent() const;                          // from MARKED onwards (C1)
    bool isMarkVisibleToStudent() const;                              // only RELEASED (FR13)

    // ---- getters
    int getId() const { return id; }
    int getAssignmentId() const { return assignmentId; }
    int getStudentId() const { return studentId; }
    std::string getFilePath() const { return filePath; }
    Timestamp getSubmittedAt() const { return submittedAt; }
    int getAttemptNo() const { return attemptNo; }
    SubmissionStatus getStatus() const { return status; }
    int getDaysLate() const { return daysLate; }
    std::optional<double> getRawMark() const { return rawMark; }
    std::optional<double> getMark() const { return mark; }
    std::string getFeedback() const { return feedback; }
    std::optional<int> getMarkedBy() const { return markedBy; }
    std::optional<Timestamp> getMarkedAt() const { return markedAt; }
    std::optional<int> getModeratedBy() const { return moderatedBy; }
    std::optional<Timestamp> getModeratedAt() const { return moderatedAt; }
    std::optional<Timestamp> getReleasedAt() const { return releasedAt; }

private:
    void transitionTo(SubmissionStatus next);

    int id = 0;
    int assignmentId;
    int studentId;
    std::string filePath;
    Timestamp submittedAt;
    int attemptNo = 1;
    SubmissionStatus status;
    int daysLate = 0;
    std::optional<double> rawMark;
    std::optional<double> mark;
    std::string feedback;
    std::optional<int> markedBy;
    std::optional<Timestamp> markedAt;
    std::optional<int> moderatedBy;
    std::optional<Timestamp> moderatedAt;
    std::optional<Timestamp> releasedAt;
};
