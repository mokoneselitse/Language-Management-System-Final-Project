#include "User.h"

#include <utility>

User::User(int id, std::string username, std::string fullName, std::string passwordHash,
           bool active)
    : id(id),
      username(std::move(username)),
      passwordHash(std::move(passwordHash)),
      fullName(std::move(fullName)),
      active(active) {}

int User::getId() const { return id; }
std::string User::getUsername() const { return username; }
std::string User::getFullName() const { return fullName; }
std::string User::getPasswordHash() const { return passwordHash; }
bool User::isActive() const { return active; }

// ---------------------------------------------------------------- Administrator
Role Administrator::getRole() const { return Role::ADMIN; }

bool Administrator::canPerform(Action action) const {
    return action == Action::MANAGE_USERS || action == Action::MANAGE_COURSES ||
           action == Action::ASSIGN_LECTURER || action == Action::ENROL_STUDENT ||
           action == Action::VIEW_AUDIT_LOG;
}

// ---------------------------------------------------------------- Lecturer
Role Lecturer::getRole() const { return Role::LECTURER; }

bool Lecturer::canPerform(Action action) const {
    return action == Action::CREATE_ASSIGNMENT || action == Action::GRANT_EXTENSION ||
           action == Action::MARK_SUBMISSION || action == Action::VIEW_OWN_ASSIGNMENTS;
}

// ---------------------------------------------------------------- Student
Role Student::getRole() const { return Role::STUDENT; }

bool Student::canPerform(Action action) const {
    return action == Action::VIEW_COURSES || action == Action::VIEW_ASSIGNMENTS ||
           action == Action::SUBMIT_ASSIGNMENT || action == Action::VIEW_RESULTS;
}

// ---------------------------------------------------------------- Head of Department
Role HeadOfDepartment::getRole() const { return Role::HOD; }

bool HeadOfDepartment::canPerform(Action action) const {
    return action == Action::REVIEW_MARKS || action == Action::MODERATE_MARK ||
           action == Action::RELEASE_MARKS || action == Action::VIEW_AUDIT_LOG;
}

std::optional<int> HeadOfDepartment::getDepartmentId() const { return departmentId; }

void HeadOfDepartment::setDepartmentId(std::optional<int> id) { departmentId = id; }
