// Unit tests for the Submission state machine (state machine diagram, Figure 5.5)
// and the C1 visibility rules (FR09, FR13).
#include <catch2/catch_test_macros.hpp>

#include "Errors.h"
#include "Submission.h"
#include "TestHelpers.h"

static Submission onTime() {
    return Submission(1, 2, "file.pdf", testDeadline(), SubmissionStatus::SUBMITTED, 0);
}
static Submission late() {
    return Submission(1, 2, "file.pdf", testDeadline(), SubmissionStatus::LATE, 2);
}

TEST_CASE("UT-10 A new submission can only start as SUBMITTED or LATE", "[unit][state]") {
    CHECK_NOTHROW(onTime());
    CHECK_NOTHROW(late());
    CHECK_THROWS_AS(Submission(1, 2, "f", testDeadline(), SubmissionStatus::MARKED, 0),
                    InvalidTransitionError);
    CHECK_THROWS_AS(Submission(1, 2, "f", testDeadline(), SubmissionStatus::RELEASED, 0),
                    InvalidTransitionError);
}

TEST_CASE("UT-11 Full happy path SUBMITTED -> MARKED -> MODERATED -> RELEASED",
          "[unit][state][FR07][FR11][FR12]") {
    Timestamp t = testDeadline();
    Submission s = onTime();
    s.recordMark(40, 40, "Good", 10, t);
    CHECK(s.getStatus() == SubmissionStatus::MARKED);
    s.approve(20, t);
    CHECK(s.getStatus() == SubmissionStatus::MODERATED);
    CHECK(s.getModeratedBy() == 20);
    s.release(t);
    CHECK(s.getStatus() == SubmissionStatus::RELEASED);
    CHECK(s.getReleasedAt().has_value());
}

TEST_CASE("UT-12 Resubmission is allowed only while SUBMITTED", "[unit][state][FR17]") {
    Timestamp t = testDeadline();
    Submission s = onTime();
    s.resubmit("v2.pdf", t);
    CHECK(s.getAttemptNo() == 2);
    CHECK(s.getFilePath() == "v2.pdf");
    CHECK(s.getStatus() == SubmissionStatus::SUBMITTED);

    Submission l = late();
    CHECK_THROWS_AS(l.resubmit("v2.pdf", t), InvalidTransitionError);  // O2: no late resubmit

    s.recordMark(30, 30, "ok", 10, t);
    CHECK_THROWS_AS(s.resubmit("v3.pdf", t), InvalidTransitionError);  // already marked
}

TEST_CASE("UT-13 Late work can be marked; re-marking keeps it MARKED", "[unit][state][FR07]") {
    Timestamp t = testDeadline();
    Submission s = late();
    s.recordMark(40, 30, "late", 10, t);
    CHECK(s.getStatus() == SubmissionStatus::MARKED);
    CHECK(s.getRawMark() == 40.0);
    CHECK(s.getMark() == 30.0);
    s.recordMark(45, 35, "late", 10, t);  // re-mark (self-transition)
    CHECK(s.getStatus() == SubmissionStatus::MARKED);
    CHECK(s.getMark() == 35.0);
}

TEST_CASE("UT-14 There is no MARKED -> RELEASED transition (C1)", "[unit][state][FR13]") {
    Timestamp t = testDeadline();
    Submission s = onTime();
    s.recordMark(40, 40, "Good", 10, t);
    CHECK_THROWS_AS(s.release(t), InvalidTransitionError);
}

TEST_CASE("UT-15 Invalid transitions are rejected", "[unit][state]") {
    Timestamp t = testDeadline();
    Submission s = onTime();
    CHECK_THROWS_AS(s.approve(20, t), InvalidTransitionError);       // not marked yet
    CHECK_THROWS_AS(s.adjustMark(30, 20, t), InvalidTransitionError);
    CHECK_THROWS_AS(s.release(t), InvalidTransitionError);

    s.recordMark(40, 40, "Good", 10, t);
    s.approve(20, t);
    CHECK_THROWS_AS(s.approve(20, t), InvalidTransitionError);       // approve only from MARKED
    CHECK_THROWS_AS(s.recordMark(10, 10, "x", 10, t), InvalidTransitionError);
    CHECK_NOTHROW(s.adjustMark(38, 20, t));                          // MODERATED -> MODERATED

    s.release(t);
    CHECK_THROWS_AS(s.adjustMark(30, 20, t), InvalidTransitionError);  // RELEASED is final
    CHECK_THROWS_AS(s.recordMark(10, 10, "x", 10, t), InvalidTransitionError);
    CHECK_THROWS_AS(s.release(t), InvalidTransitionError);
}

TEST_CASE("UT-16 Feedback is visible from MARKED, the mark only when RELEASED",
          "[unit][state][FR09][FR13]") {
    Timestamp t = testDeadline();
    Submission s = onTime();
    CHECK_FALSE(s.isFeedbackVisibleToStudent());
    CHECK_FALSE(s.isMarkVisibleToStudent());

    s.recordMark(40, 40, "Good", 10, t);
    CHECK(s.isFeedbackVisibleToStudent());
    CHECK_FALSE(s.isMarkVisibleToStudent());

    s.approve(20, t);
    CHECK(s.isFeedbackVisibleToStudent());
    CHECK_FALSE(s.isMarkVisibleToStudent());

    s.release(t);
    CHECK(s.isFeedbackVisibleToStudent());
    CHECK(s.isMarkVisibleToStudent());
}
