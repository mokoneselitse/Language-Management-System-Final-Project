// Integration tests: login, user management and course setup (FR01 - FR05, FR14, FR20, NFR04)
// against a real SQLite database.
#include <catch2/catch_test_macros.hpp>

#include "Errors.h"
#include "LmsFixture.h"

TEST_CASE_METHOD(LmsFixture, "IT-01 Login accepts correct details and rejects wrong ones",
                 "[integration][auth][FR20]") {
    CHECK(login("lec1").user().getRole() == Role::LECTURER);
    CHECK_THROWS_AS(auth.login("lec1", "wrong"), AuthenticationError);
    CHECK_THROWS_AS(auth.login("nobody", "pw"), AuthenticationError);
    CHECK_THROWS_AS(auth.login("", ""), AuthenticationError);
}

TEST_CASE_METHOD(LmsFixture, "IT-02 Passwords are stored only as Argon2id hashes",
                 "[integration][security][NFR04]") {
    // A long, unique password, so it cannot appear inside a hash by chance.
    // (An earlier version checked for "pw" and failed randomly - see the defect log.)
    const std::string password = "Plain-Text-Secret-2026";
    int id = admin.createUser(*adminSession, Role::STUDENT, "stu9", "Student Nine", password);

    Statement st = db.prepare("SELECT password_hash FROM users;");
    int rows = 0;
    while (st.step()) {
        std::string hash = st.columnText(0);
        CHECK(hash.rfind("$argon2id$", 0) == 0);  // every user, Argon2id only
        rows++;
    }
    CHECK(rows == 8);

    std::string stored = users.findById(id)->getPasswordHash();
    CHECK(stored != password);
    CHECK(stored.find(password) == std::string::npos);
    CHECK(hasher.verify(password, stored));  // but the password still works at login
    // The database itself refuses a plain-text password (CHECK constraint).
    CHECK_THROWS_AS(users.add(Role::STUDENT, "plain", "Plain", "Password1"), DatabaseError);
}

TEST_CASE_METHOD(LmsFixture, "IT-03 Admin creates, edits and deactivates users",
                 "[integration][admin][FR01]") {
    Session& s = *adminSession;
    CHECK_THROWS_AS(admin.createUser(s, Role::STUDENT, "stu1", "Duplicate", "pw"),
                    ValidationError);
    CHECK_THROWS_AS(admin.createUser(s, Role::STUDENT, "ab", "Too short", "pw"), ValidationError);

    admin.editUser(s, studentId, "stu1new", "Student Renamed");
    CHECK(login("stu1new").user().getFullName() == "Student Renamed");
    CHECK_THROWS_AS(admin.editUser(s, studentId, "lec1", "Taken"), ValidationError);

    admin.resetPassword(s, studentId, "newpw");
    CHECK_THROWS_AS(login("stu1new"), AuthenticationError);
    CHECK(auth.login("stu1new", "newpw").user().getId() == studentId);

    admin.setUserActive(s, studentId, false);  // deactivated users cannot log in
    CHECK_THROWS_AS(auth.login("stu1new", "newpw"), AuthenticationError);
    admin.setUserActive(s, studentId, true);
    CHECK_NOTHROW(auth.login("stu1new", "newpw"));
    CHECK_THROWS_AS(admin.setUserActive(s, s.user().getId(), false), ValidationError);
}

TEST_CASE_METHOD(LmsFixture, "IT-04 Admin manages departments and courses",
                 "[integration][admin][FR02]") {
    Session& s = *adminSession;
    CHECK(admin.listDepartments(s).size() == 2);
    // a HoD can head only one department
    CHECK_THROWS_AS(admin.createDepartment(s, "Physics", hodId), ValidationError);
    CHECK_THROWS_AS(admin.createDepartment(s, "Physics", lecturerId), ValidationError);
    CHECK_THROWS_AS(admin.setDepartmentHod(s, departmentId, otherHodId), ValidationError);

    CHECK_THROWS_AS(admin.createCourse(s, "CS4433", "Duplicate code", departmentId),
                    ValidationError);
    CHECK_THROWS_AS(admin.createCourse(s, "CS9999", "No such dept", 999), ValidationError);
    admin.editCourse(s, courseId, "CS4433A", "Software Engineering II");
    CHECK(courses.findCourse(courseId)->getCode() == "CS4433A");
    CHECK(admin.listCourses(s).size() == 1);
}

TEST_CASE_METHOD(LmsFixture, "IT-05 Lecturer assignment and enrolment show up in the lists",
                 "[integration][admin][FR03][FR04][FR14]") {
    Session lec = login("lec1");
    CHECK(assignmentService.coursesForLecturer(lec).size() == 1);
    Session stu = login("stu1");
    REQUIRE(assignmentService.coursesForStudent(stu).size() == 1);
    CHECK(assignmentService.coursesForStudent(stu)[0].getCode() == "CS4433");

    CHECK(auth.nameOf(lec, studentId) == "Student One");  // menus show names, not ids
    CHECK(auth.nameOf(lec, 999).find("unknown") != std::string::npos);

    CHECK_THROWS_AS(admin.assignLecturer(*adminSession, courseId, studentId), ValidationError);
    CHECK_THROWS_AS(admin.enrolStudent(*adminSession, studentId, courseId), ValidationError);
    CHECK_THROWS_AS(admin.enrolStudent(*adminSession, lecturerId, courseId), ValidationError);
}

TEST_CASE_METHOD(LmsFixture, "IT-06 Lecturer creates assignments only for their own course",
                 "[integration][assignment][FR05][FR15][FR23]") {
    int a = createAssignment();
    Session stu = login("stu1");
    REQUIRE(assignmentService.listForStudent(stu).size() == 1);
    CHECK(assignmentService.listForStudent(stu)[0].getMaxMark() == 50);
    CHECK(assignmentService.listForStudent(stu)[0].getDeadline() == testDeadline());
    Session lecView = login("lec1");
    CHECK(assignmentService.listForLecturer(lecView, true)[0].getId() == a);

    Session other = login("lec2");
    CHECK_THROWS_AS(assignmentService.createAssignment(other, courseId, "X", "", testDeadline(), 50),
                    AccessDeniedError);
    Session lec = login("lec1");
    CHECK_THROWS_AS(assignmentService.createAssignment(lec, courseId, "X", "", testDeadline(), 0),
                    ValidationError);
    CHECK_THROWS_AS(
        assignmentService.createAssignment(lec, courseId, "X", "", clock.now() - hours(1), 50),
        ValidationError);
    CHECK_THROWS_AS(assignmentService.createAssignment(lec, courseId, " ", "", testDeadline(), 50),
                    ValidationError);
}
