// Integration tests: the full submit -> mark -> moderate -> release workflow
// (FR06 - FR13, FR16 - FR19, FR22) against a real SQLite database.
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Errors.h"
#include "LmsFixture.h"

using Catch::Approx;

TEST_CASE_METHOD(LmsFixture, "IT-07 On-time submission, resubmission and file storage",
                 "[integration][submit][FR16][FR17]") {
    int a = createAssignment();
    Session stu = login("stu1");
    Submission first = submissionService.submit(stu, a, workFile);
    CHECK(first.getStatus() == SubmissionStatus::SUBMITTED);
    CHECK(first.getAttemptNo() == 1);
    CHECK(fs::exists(first.getFilePath()));  // the file was copied into storage

    clock.set(testDeadline());  // resubmitting exactly at the deadline is still allowed
    Submission second = submissionService.submit(stu, a, workFile);
    CHECK(second.getAttemptNo() == 2);
    CHECK(second.getId() == first.getId());  // same row, overwritten (FR17)
    CHECK(fs::exists(first.getFilePath()));  // the first attempt's copy is kept

    CHECK_THROWS_AS(submissionService.submit(stu, a, "no_such_file.txt"), ValidationError);
    CHECK_THROWS_AS(submissionService.submit(stu, 999, workFile), NotFoundError);
}

TEST_CASE_METHOD(LmsFixture, "IT-08 Late work is accepted with days late; late resubmission is refused",
                 "[integration][submit][FR16][FR17][C2][O2]") {
    int a = createAssignment();
    Session s1 = login("stu1");
    submissionService.submit(s1, a, workFile);  // on time

    clock.set(testDeadline() + hours(30));  // 1 day 6 hours late -> 2 days
    Session s2 = login("stu2");
    Submission late = submissionService.submit(s2, a, workFile);
    CHECK(late.getStatus() == SubmissionStatus::LATE);
    CHECK(late.getDaysLate() == 2);

    CHECK_THROWS_AS(submissionService.submit(s2, a, workFile), ValidationError);  // late resubmit
    CHECK_THROWS_AS(submissionService.submit(s1, a, workFile), ValidationError);  // after deadline
}

TEST_CASE_METHOD(LmsFixture, "IT-09 An extension lets only that student submit on time later",
                 "[integration][extension][FR06]") {
    int a = createAssignment();
    Session lec = login("lec1");
    CHECK_THROWS_AS(assignmentService.grantExtension(lec, a, studentId, testDeadline() - hours(1),
                                                     "earlier"),
                    ValidationError);
    CHECK_THROWS_AS(assignmentService.grantExtension(lec, a, studentId,
                                                     testDeadline() + hours(48), "  "),
                    ValidationError);
    assignmentService.grantExtension(lec, a, studentId, testDeadline() + hours(48), "Sick note");

    clock.set(testDeadline() + hours(24));
    Session s1 = login("stu1");
    Session s2 = login("stu2");
    CHECK(submissionService.submit(s1, a, workFile).getStatus() == SubmissionStatus::SUBMITTED);
    CHECK(submissionService.submit(s2, a, workFile).getStatus() == SubmissionStatus::LATE);
    CHECK(assignmentService.myExtension(s1, a).has_value());
    CHECK_FALSE(assignmentService.myExtension(s2, a).has_value());
}

TEST_CASE_METHOD(LmsFixture, "IT-10 Marking applies the late penalty and writes an audit entry",
                 "[integration][marking][FR07][FR08][FR18][FR22]") {
    int a = createAssignment();
    Session s1 = login("stu1");
    int onTimeId = submissionService.submit(s1, a, workFile).getId();
    clock.set(testDeadline() + hours(30));  // 2 days late
    Session s2 = login("stu2");
    int lateId = submissionService.submit(s2, a, workFile).getId();

    Session lec = login("lec1");
    CHECK(marking.submissionsFor(lec, a).size() == 2);
    CHECK_THROWS_AS(marking.mark(lec, onTimeId, 51, "too high"), ValidationError);
    CHECK_THROWS_AS(marking.mark(lec, onTimeId, -1, "negative"), ValidationError);
    CHECK_THROWS_AS(marking.mark(lec, onTimeId, 40, " "), ValidationError);  // feedback required

    marking.mark(lec, onTimeId, 40, "Good");
    marking.mark(lec, lateId, 40, "Late");
    CHECK(submissions.findById(onTimeId)->getMark() == Approx(40));
    CHECK(submissions.findById(lateId)->getRawMark() == Approx(40));
    CHECK(submissions.findById(lateId)->getMark() == Approx(30));  // 40 - 2 x 5

    std::vector<AuditEntry> log = audit.forSubmission(lateId);
    REQUIRE(log.size() == 1);
    CHECK(log[0].getAction() == AuditAction::MARK);
    CHECK(log[0].getChangedBy() == lecturerId);
    CHECK_FALSE(log[0].getOldMark().has_value());
    CHECK(log[0].getNewMark() == Approx(30));

    CHECK_THROWS_AS(marking.mark(lec, onTimeId, 45, "again"), InvalidTransitionError);
}

TEST_CASE_METHOD(LmsFixture, "IT-11 Re-marking needs a reason and records old and new marks",
                 "[integration][marking][FR22]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();
    Session lec = login("lec1");
    CHECK_THROWS_AS(marking.remark(lec, id, 45, "not marked yet"), InvalidTransitionError);
    marking.mark(lec, id, 40, "Good");
    CHECK_THROWS_AS(marking.remark(lec, id, 45, ""), ValidationError);
    marking.remark(lec, id, 45, "Missed a section");

    std::vector<AuditEntry> log = audit.forSubmission(id);
    REQUIRE(log.size() == 2);
    CHECK(log[1].getAction() == AuditAction::REMARK);
    CHECK(log[1].getOldMark() == Approx(40));
    CHECK(log[1].getNewMark() == Approx(45));
    CHECK(log[1].getReason() == "Missed a section");
    CHECK(submissions.findById(id)->getFeedback() == "Good");  // feedback kept
}

TEST_CASE_METHOD(LmsFixture, "IT-12 Students see feedback after marking but the mark only after release",
                 "[integration][visibility][FR09][FR13][FR19][C1]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();

    std::vector<StudentResult> r = submissionService.resultsFor(stu);
    REQUIRE(r.size() == 1);
    CHECK_FALSE(r[0].feedback.has_value());
    CHECK_FALSE(r[0].mark.has_value());

    Session lec = login("lec1");
    marking.mark(lec, id, 40, "Good structure");
    r = submissionService.resultsFor(stu);
    CHECK(r[0].feedback == "Good structure");  // FR09
    CHECK_FALSE(r[0].mark.has_value());        // FR13

    Session hod = login("hod1");
    moderation.approve(hod, id);
    r = submissionService.resultsFor(stu);
    CHECK_FALSE(r[0].mark.has_value());  // still hidden while MODERATED

    moderation.release(hod, a);
    r = submissionService.resultsFor(stu);
    CHECK(r[0].status == SubmissionStatus::RELEASED);
    CHECK(r[0].mark == Approx(40));  // FR19
}

TEST_CASE_METHOD(LmsFixture, "IT-13 The HoD sees only marked work from their own department",
                 "[integration][moderation][FR10]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();
    Session hod = login("hod1");
    CHECK(moderation.pendingForDepartment(hod).empty());  // not marked yet

    Session lec = login("lec1");
    marking.mark(lec, id, 40, "Good");
    CHECK(moderation.pendingForDepartment(hod).size() == 1);
    Session other = login("hod2");
    CHECK(moderation.pendingForDepartment(other).empty());  // different department
}

TEST_CASE_METHOD(LmsFixture, "IT-14 Adjusting a mark needs a reason and a valid mark, and is audited",
                 "[integration][moderation][FR11][FR22]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();
    Session lec = login("lec1");
    marking.mark(lec, id, 40, "Good");

    Session hod = login("hod1");
    CHECK_THROWS_AS(moderation.adjust(hod, id, 42, ""), ValidationError);  // FR11
    CHECK_THROWS_AS(moderation.adjust(hod, id, 60, "too high"), ValidationError);
    moderation.adjust(hod, id, 42, "Generous on testing");
    moderation.adjust(hod, id, 41, "Second look");  // MODERATED -> MODERATED

    std::vector<AuditEntry> log = audit.forSubmission(id);
    REQUIRE(log.size() == 3);
    CHECK(log[1].getAction() == AuditAction::MODERATE);
    CHECK(log[1].getChangedBy() == hodId);
    CHECK(log[1].getOldMark() == Approx(40));
    CHECK(log[2].getOldMark() == Approx(42));
    CHECK(log[2].getNewMark() == Approx(41));
    CHECK(auditService.history(hod, id).size() == 3);
    CHECK(auditService.history(*adminSession, id).size() == 3);  // O1: admin can view too
}

TEST_CASE_METHOD(LmsFixture, "IT-15 Release only releases moderated work",
                 "[integration][moderation][FR12][FR13]") {
    int a = createAssignment();
    Session s1 = login("stu1");
    Session s2 = login("stu2");
    int id1 = submissionService.submit(s1, a, workFile).getId();
    int id2 = submissionService.submit(s2, a, workFile).getId();
    Session lec = login("lec1");
    marking.mark(lec, id1, 40, "Good");
    marking.mark(lec, id2, 35, "Fair");

    Session hod = login("hod1");
    moderation.approve(hod, id1);                // only id1 is moderated
    CHECK(moderation.release(hod, a) == 1);
    CHECK(submissions.findById(id1)->getStatus() == SubmissionStatus::RELEASED);
    CHECK(submissions.findById(id2)->getStatus() == SubmissionStatus::MARKED);
    CHECK_FALSE(submissionService.resultsFor(s2)[0].mark.has_value());
    CHECK(moderation.release(hod, a) == 0);      // nothing left to release
}
