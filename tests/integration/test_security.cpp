// Integration tests: access control (FR21, NFR06), transactions (FR22) and persistence.
#include <catch2/catch_test_macros.hpp>

#include "Errors.h"
#include "LmsFixture.h"

TEST_CASE_METHOD(LmsFixture, "IT-16 Every role is blocked from other roles' actions",
                 "[integration][security][FR21]") {
    int a = createAssignment();
    Session stu = login("stu1");
    Session lec = login("lec1");
    Session hod = login("hod1");
    Session& adm = *adminSession;

    CHECK_THROWS_AS(admin.createUser(stu, Role::ADMIN, "hacker", "H", "pw"), AccessDeniedError);
    CHECK_THROWS_AS(admin.createUser(lec, Role::ADMIN, "hacker", "H", "pw"), AccessDeniedError);
    CHECK_THROWS_AS(marking.mark(stu, 1, 50, "self"), AccessDeniedError);
    CHECK_THROWS_AS(moderation.release(lec, a), AccessDeniedError);
    CHECK_THROWS_AS(moderation.pendingForDepartment(stu), AccessDeniedError);
    CHECK_THROWS_AS(submissionService.submit(lec, a, workFile), AccessDeniedError);
    CHECK_THROWS_AS(submissionService.resultsFor(hod), AccessDeniedError);
    CHECK_THROWS_AS(assignmentService.createAssignment(adm, courseId, "X", "", testDeadline(), 50),
                    AccessDeniedError);
    CHECK_THROWS_AS(auditService.history(stu, 1), AccessDeniedError);
    CHECK_THROWS_AS(auditService.history(lec, 1), AccessDeniedError);
}

TEST_CASE_METHOD(LmsFixture, "IT-17 Users cannot act on work outside their own course or department",
                 "[integration][security][FR21][NFR06]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();

    Session lec2 = login("lec2");  // not this course's lecturer
    CHECK_THROWS_AS(marking.mark(lec2, id, 40, "x"), AccessDeniedError);
    CHECK_THROWS_AS(marking.submissionsFor(lec2, a), AccessDeniedError);
    CHECK_THROWS_AS(assignmentService.grantExtension(lec2, a, studentId,
                                                     testDeadline() + hours(5), "x"),
                    AccessDeniedError);

    Session lec = login("lec1");
    marking.mark(lec, id, 40, "Good");
    Session hod2 = login("hod2");  // other department
    CHECK_THROWS_AS(moderation.approve(hod2, id), AccessDeniedError);
    CHECK_THROWS_AS(moderation.adjust(hod2, id, 45, "x"), AccessDeniedError);
    CHECK_THROWS_AS(moderation.release(hod2, a), AccessDeniedError);

    // a student who is not enrolled cannot submit
    int outsider = admin.createUser(*adminSession, Role::STUDENT, "stu3", "Outsider", "pw");
    (void)outsider;
    Session s3 = login("stu3");
    CHECK_THROWS_AS(submissionService.submit(s3, a, workFile), AccessDeniedError);
}

// An audit repository that always fails, to check the transaction rolls back.
class FailingAuditRepository : public IAuditRepository {
public:
    void append(const AuditEntry&) override { throw DatabaseError("disk full"); }
    std::vector<AuditEntry> forSubmission(int) override { return {}; }
};

TEST_CASE_METHOD(LmsFixture, "IT-18 If the audit entry cannot be saved, the mark is not saved either",
                 "[integration][transaction][FR22]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();

    FailingAuditRepository failingAudit;
    MarkingService brokenMarking(submissions, assignments, failingAudit, penalty, db, clock);
    Session lec = login("lec1");
    CHECK_THROWS_AS(brokenMarking.mark(lec, id, 40, "Good"), DatabaseError);

    // rolled back: still SUBMITTED, no mark
    CHECK(submissions.findById(id)->getStatus() == SubmissionStatus::SUBMITTED);
    CHECK_FALSE(submissions.findById(id)->getMark().has_value());
}

TEST_CASE_METHOD(LmsFixture, "IT-19 The audit log cannot be changed or deleted",
                 "[integration][security][FR22]") {
    int a = createAssignment();
    Session stu = login("stu1");
    int id = submissionService.submit(stu, a, workFile).getId();
    Session lec = login("lec1");
    marking.mark(lec, id, 40, "Good");

    CHECK_THROWS_AS(db.execute("UPDATE audit_log SET new_mark = 50;"), DatabaseError);
    CHECK_THROWS_AS(db.execute("DELETE FROM audit_log;"), DatabaseError);
    CHECK(audit.forSubmission(id).size() == 1);
}

TEST_CASE("IT-20 Data is still there after the program restarts", "[integration][persistence]") {
    fs::path folder = fs::temp_directory_path() / "lms_test_persist";
    fs::remove_all(folder);
    fs::create_directories(folder);
    std::string path = (folder / "lms.db").string();
    {
        SqliteDatabase db(path);
        SqliteUserRepository users(db);
        users.add(Role::ADMIN, "admin", "Admin", "$argon2id$v=19$m=1,t=1,p=1$fake");
    }  // database closed, like quitting the program
    {
        SqliteDatabase db(path);  // reopening must not wipe the data
        SqliteUserRepository users(db);
        REQUIRE(users.findByUsername("admin") != nullptr);
        CHECK(users.findByUsername("admin")->getFullName() == "Admin");
    }
    fs::remove_all(folder);
}

TEST_CASE_METHOD(LmsFixture, "IT-21 The database rejects rows that break the life-cycle rules",
                 "[integration][database][state]") {
    int a = createAssignment();
    std::string ids = std::to_string(a) + ", " + std::to_string(studentId);
    // LATE with 0 days late
    CHECK_THROWS_AS(db.execute("INSERT INTO submission (assignment_id, student_id, file_path, "
                               "submitted_at, status, days_late) VALUES (" + ids +
                               ", 'f', 1, 'LATE', 0);"),
                    DatabaseError);
    // RELEASED without a release time
    CHECK_THROWS_AS(db.execute("INSERT INTO submission (assignment_id, student_id, file_path, "
                               "submitted_at, status, raw_mark, mark, marked_by, marked_at) "
                               "VALUES (" + ids + ", 'f', 1, 'RELEASED', 1, 1, 1, 1);"),
                    DatabaseError);
    // unknown status
    CHECK_THROWS_AS(db.execute("INSERT INTO submission (assignment_id, student_id, file_path, "
                               "submitted_at, status) VALUES (" + ids + ", 'f', 1, 'DONE');"),
                    DatabaseError);
}
