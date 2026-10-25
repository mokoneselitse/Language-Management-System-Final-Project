#include "AdminService.h"

#include "Errors.h"
#include "Validation.h"

AdminService::AdminService(IUserRepository& users, ICourseRepository& courses,
                           IPasswordHasher& hasher, IClock& clock)
    : users(users), courses(courses), hasher(hasher), clock(clock) {}

int AdminService::createUser(Session& s, Role role, const std::string& username,
                             const std::string& name, const std::string& pw) {
    s.require(Action::MANAGE_USERS);
    if (username.size() < 3 || username.size() > 32 ||
        username.find(' ') != std::string::npos) {
        throw ValidationError("Username must be 3-32 characters with no spaces.");
    }
    requireNotBlank(name, "Full name");
    requireNotBlank(pw, "Password");
    if (users.findByUsername(username) != nullptr) {
        throw ValidationError("Username '" + username + "' is already taken.");
    }
    // Only the hash is stored, never the password itself (NFR04).
    return users.add(role, username, name, hasher.hash(pw));
}

void AdminService::setUserActive(Session& s, int userId, bool active) {
    s.require(Action::MANAGE_USERS);
    if (users.findById(userId) == nullptr) throw NotFoundError("No user with that id.");
    if (userId == s.user().getId() && !active) {
        throw ValidationError("You cannot deactivate your own account.");
    }
    users.setActive(userId, active);
}

int AdminService::createDepartment(Session& s, const std::string& name,
                                   std::optional<int> hodId) {
    s.require(Action::MANAGE_COURSES);
    requireNotBlank(name, "Department name");
    if (hodId) {
        std::unique_ptr<User> hod = users.findById(*hodId);
        if (hod == nullptr || hod->getRole() != Role::HOD) {
            throw ValidationError("That user is not a Head of Department.");
        }
        if (courses.departmentOfHod(*hodId)) {
            throw ValidationError("That Head of Department already heads a department.");
        }
    }
    try {
        return courses.addDepartment(name, hodId);
    } catch (const DatabaseError&) {
        throw ValidationError("A department with that name already exists.");
    }
}

int AdminService::createCourse(Session& s, const std::string& code, const std::string& title,
                               int departmentId) {
    s.require(Action::MANAGE_COURSES);
    requireNotBlank(code, "Course code");
    requireNotBlank(title, "Course title");
    try {
        return courses.addCourse(code, title, departmentId);
    } catch (const DatabaseError&) {
        throw ValidationError("Course code already used, or the department does not exist.");
    }
}

void AdminService::assignLecturer(Session& s, int courseId, int lecturerId) {
    s.require(Action::ASSIGN_LECTURER);
    if (!courses.findCourse(courseId)) throw NotFoundError("No course with that id.");
    std::unique_ptr<User> lecturer = users.findById(lecturerId);
    if (lecturer == nullptr || lecturer->getRole() != Role::LECTURER) {
        throw ValidationError("That user is not a Lecturer.");
    }
    courses.assignLecturer(courseId, lecturerId);
}

void AdminService::enrolStudent(Session& s, int studentId, int courseId) {
    s.require(Action::ENROL_STUDENT);
    if (!courses.findCourse(courseId)) throw NotFoundError("No course with that id.");
    std::unique_ptr<User> student = users.findById(studentId);
    if (student == nullptr || student->getRole() != Role::STUDENT) {
        throw ValidationError("That user is not a Student.");
    }
    if (courses.isEnrolled(studentId, courseId)) {
        throw ValidationError("The student is already enrolled on this course.");
    }
    courses.enrol(studentId, courseId, clock.now());
}
