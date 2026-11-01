// LmsFixture.h - builds the whole system (real SQLite database, real
// repositories and services) for the integration tests, the same way main() does.
// Each test gets its own fresh database and storage folder in a temp directory.
#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "AdminService.h"
#include "AssignmentService.h"
#include "AuditService.h"
#include "AuthService.h"
#include "MarkingService.h"
#include "ModerationService.h"
#include "SodiumPasswordHasher.h"
#include "SqliteRepositories.h"
#include "SubmissionService.h"
#include "TestHelpers.h"

namespace fs = std::filesystem;

// A temporary folder that deletes itself. It is the fixture's FIRST member, so it
// is destroyed LAST - after the database is closed (Windows cannot delete open files).
struct TempFolder {
    fs::path path;
    TempFolder() {
        static int counter = 0;
        auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() /
               ("lms_test_" + std::to_string(stamp) + "_" + std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempFolder() {
        std::error_code ignored;
        fs::remove_all(path, ignored);
    }
};

class LmsFixture {
public:
    LmsFixture()
        : temp(),
          folder(temp.path),
          db((folder / "test.db").string()),
          users(db),
          courses(db),
          assignments(db),
          submissions(db),
          audit(db),
          hasher(1, 8192),  // Argon2id at libsodium's minimum cost, so the tests run fast
          clock(testDeadline() - hours(48)),  // "now" starts 2 days before the deadline
          auth(users, hasher),
          admin(users, courses, hasher, clock),
          assignmentService(assignments, courses, clock),
          submissionService(assignments, courses, submissions, clock,
                            (folder / "submissions").string()),
          marking(submissions, assignments, audit, penalty, db, clock),
          moderation(submissions, assignments, courses, audit, db, clock),
          auditService(audit) {
        // Standard test data: one department with a HoD, one course with a
        // lecturer, two enrolled students, and a second department.
        users.add(Role::ADMIN, "admin", "Admin", hasher.hash("adminpw"));
        adminSession = std::make_unique<Session>(auth.login("admin", "adminpw"));
        lecturerId = admin.createUser(*adminSession, Role::LECTURER, "lec1", "Lecturer One", "pw");
        otherLecturerId =
            admin.createUser(*adminSession, Role::LECTURER, "lec2", "Lecturer Two", "pw");
        hodId = admin.createUser(*adminSession, Role::HOD, "hod1", "HoD One", "pw");
        otherHodId = admin.createUser(*adminSession, Role::HOD, "hod2", "HoD Two", "pw");
        studentId = admin.createUser(*adminSession, Role::STUDENT, "stu1", "Student One", "pw");
        student2Id = admin.createUser(*adminSession, Role::STUDENT, "stu2", "Student Two", "pw");
        departmentId = admin.createDepartment(*adminSession, "Computer Science", hodId);
        admin.createDepartment(*adminSession, "Mathematics", otherHodId);
        courseId = admin.createCourse(*adminSession, "CS4433", "Software Engineering", departmentId);
        admin.assignLecturer(*adminSession, courseId, lecturerId);
        admin.enrolStudent(*adminSession, studentId, courseId);
        admin.enrolStudent(*adminSession, student2Id, courseId);
        workFile = (folder / "essay.txt").string();
        std::ofstream(workFile) << "my essay";
    }

    Session login(const std::string& username) { return auth.login(username, "pw"); }

    // Creates the standard assignment (max mark 50, deadline = testDeadline()).
    int createAssignment() {
        Session lec = login("lec1");
        return assignmentService.createAssignment(lec, courseId, "Project", "Build it",
                                                  testDeadline(), 50);
    }

    TempFolder temp;
    fs::path folder;
    SqliteDatabase db;
    SqliteUserRepository users;
    SqliteCourseRepository courses;
    SqliteAssignmentRepository assignments;
    SqliteSubmissionRepository submissions;
    SqliteAuditRepository audit;
    SodiumPasswordHasher hasher;
    FixedClock clock;
    StandardLatePenalty penalty;
    AuthService auth;
    AdminService admin;
    AssignmentService assignmentService;
    SubmissionService submissionService;
    MarkingService marking;
    ModerationService moderation;
    AuditService auditService;

    std::unique_ptr<Session> adminSession;
    int lecturerId = 0, otherLecturerId = 0, hodId = 0, otherHodId = 0;
    int studentId = 0, student2Id = 0, departmentId = 0, courseId = 0;
    std::string workFile;

};
