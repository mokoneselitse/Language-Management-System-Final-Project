// Unit tests for the User hierarchy and role permissions (FR21),
// UserFactory, Session::require and the Role/Status text conversions.
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <set>
#include <vector>

#include "Errors.h"
#include "Session.h"
#include "UserFactory.h"

static const std::vector<Action> ALL_ACTIONS = {
    Action::VIEW_COURSES,      Action::VIEW_ASSIGNMENTS,     Action::SUBMIT_ASSIGNMENT,
    Action::VIEW_RESULTS,      Action::CREATE_ASSIGNMENT,    Action::GRANT_EXTENSION,
    Action::MARK_SUBMISSION,   Action::VIEW_OWN_ASSIGNMENTS, Action::REVIEW_MARKS,
    Action::MODERATE_MARK,     Action::RELEASE_MARKS,        Action::VIEW_AUDIT_LOG,
    Action::MANAGE_USERS,      Action::MANAGE_COURSES,       Action::ASSIGN_LECTURER,
    Action::ENROL_STUDENT};

// Checks that the user can do exactly the allowed actions and nothing else.
static void checkPermissions(const User& user, const std::set<Action>& allowed) {
    for (Action a : ALL_ACTIONS) {
        INFO("action number " << static_cast<int>(a));
        CHECK(user.canPerform(a) == (allowed.count(a) == 1));
    }
}

TEST_CASE("UT-17 Each role can perform exactly its own actions", "[unit][roles][FR21]") {
    SECTION("Student") {
        Student u(1, "s", "S", "h", true);
        checkPermissions(u, {Action::VIEW_COURSES, Action::VIEW_ASSIGNMENTS,
                             Action::SUBMIT_ASSIGNMENT, Action::VIEW_RESULTS});
    }
    SECTION("Lecturer") {
        Lecturer u(1, "l", "L", "h", true);
        checkPermissions(u, {Action::CREATE_ASSIGNMENT, Action::GRANT_EXTENSION,
                             Action::MARK_SUBMISSION, Action::VIEW_OWN_ASSIGNMENTS});
    }
    SECTION("Head of Department") {
        HeadOfDepartment u(1, "h", "H", "h", true);
        checkPermissions(u, {Action::REVIEW_MARKS, Action::MODERATE_MARK, Action::RELEASE_MARKS,
                             Action::VIEW_AUDIT_LOG});
    }
    SECTION("Administrator") {
        Administrator u(1, "a", "A", "h", true);
        checkPermissions(u, {Action::MANAGE_USERS, Action::MANAGE_COURSES,
                             Action::ASSIGN_LECTURER, Action::ENROL_STUDENT,
                             Action::VIEW_AUDIT_LOG});
    }
}

TEST_CASE("UT-18 UserFactory creates the right subclass for each role", "[unit][roles][factory]") {
    for (Role r : {Role::STUDENT, Role::LECTURER, Role::HOD, Role::ADMIN}) {
        std::unique_ptr<User> u = UserFactory::create(r, 7, "user", "Full Name", "hash", true);
        REQUIRE(u != nullptr);
        CHECK(u->getRole() == r);
        CHECK(u->getId() == 7);
        CHECK(u->getFullName() == "Full Name");
    }
    std::unique_ptr<User> hod = UserFactory::create(Role::HOD, 1, "h", "H", "x", true);
    CHECK(dynamic_cast<HeadOfDepartment*>(hod.get()) != nullptr);
}

TEST_CASE("UT-19 Session::require throws AccessDeniedError for a forbidden action",
          "[unit][roles][FR21]") {
    Session s(std::make_unique<Student>(1, "s", "S", "h", true));
    CHECK_NOTHROW(s.require(Action::SUBMIT_ASSIGNMENT));
    CHECK_THROWS_AS(s.require(Action::MARK_SUBMISSION), AccessDeniedError);
    CHECK_THROWS_AS(s.require(Action::MANAGE_USERS), AccessDeniedError);
}

TEST_CASE("UT-20 Enum text conversions round-trip and reject unknown text", "[unit][types]") {
    for (Role r : {Role::STUDENT, Role::LECTURER, Role::HOD, Role::ADMIN}) {
        CHECK(roleFromString(toString(r)) == r);
    }
    for (SubmissionStatus st : {SubmissionStatus::SUBMITTED, SubmissionStatus::LATE,
                                SubmissionStatus::MARKED, SubmissionStatus::MODERATED,
                                SubmissionStatus::RELEASED}) {
        CHECK(statusFromString(toString(st)) == st);
    }
    CHECK(auditActionFromString("REMARK") == AuditAction::REMARK);
    CHECK_THROWS_AS(roleFromString("TEACHER"), ValidationError);
    CHECK_THROWS_AS(statusFromString("DONE"), ValidationError);
}

TEST_CASE("UT-21 Timestamps convert to Unix seconds and back", "[unit][types]") {
    CHECK(toUnixSeconds(fromUnixSeconds(1800000000)) == 1800000000);
}
