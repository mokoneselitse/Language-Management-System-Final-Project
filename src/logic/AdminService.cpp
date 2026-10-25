#include "AdminService.h"

#include "Errors.h"
#include "Validation.h"

AdminService::AdminService(IUserRepository& users, ICourseRepository& courses,
                           IPasswordHasher& hasher, IClock& clock)
    : users(users), courses(courses), hasher(hasher), clock(clock) {}

void AdminService::checkUsername(const std::string& username) {
    if (username.size() < 3 || username.size() > 32 ||
        username.find(' ') != std::string::npos) {
        throw ValidationError("Username must be 3-32 characters with no spaces.");
    }
}

// The user must be a HoD who does not already head another department.
// allowedDepartmentId is the department being edited (0 when creating one).
void AdminService::checkCanBeHod(int hodId, int allowedDepartmentId) {
    std::unique_ptr<User> hod = users.findById(hodId);
    if (hod == nullptr || hod->getRole() != Role::HOD) {
        throw ValidationError("That user is not a Head of Department.");
    }
    std::optional<int> current = courses.departmentOfHod(hodId);
    if (current && *current != allowedDepartmentId) {
        throw ValidationError("That Head of Department already heads a department.");
    }
}

int AdminService::createUser(Session& s, Role role, const std::string& username,
                             const std::string& name, const std::string& pw) {
    s.require(Action::MANAGE_USERS);
    checkUsername(username);
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
    if (hodId) checkCanBeHod(*hodId, 0);
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

void AdminService::editUser(Session& s, int userId, const std::string& username,
                            const std::string& name) {
    s.require(Action::MANAGE_USERS);
    std::unique_ptr<User> user = users.findById(userId);
    if (user == nullptr) throw NotFoundError("No user with that id.");
    checkUsername(username);
    requireNotBlank(name, "Full name");
    std::unique_ptr<User> sameName = users.findByUsername(username);
    if (sameName != nullptr && sameName->getId() != userId) {
        throw ValidationError("Username '" + username + "' is already taken.");
    }
    users.update(userId, username, name);
}

void AdminService::resetPassword(Session& s, int userId, const std::string& newPassword) {
    s.require(Action::MANAGE_USERS);
    if (users.findById(userId) == nullptr) throw NotFoundError("No user with that id.");
    requireNotBlank(newPassword, "Password");
    users.setPasswordHash(userId, hasher.hash(newPassword));
}

void AdminService::editCourse(Session& s, int courseId, const std::string& code,
                              const std::string& title) {
    s.require(Action::MANAGE_COURSES);
    if (!courses.findCourse(courseId)) throw NotFoundError("No course with that id.");
    requireNotBlank(code, "Course code");
    requireNotBlank(title, "Course title");
    try {
        courses.updateCourse(courseId, code, title);
    } catch (const DatabaseError&) {
        throw ValidationError("Another course already uses that code.");
    }
}

void AdminService::setDepartmentHod(Session& s, int departmentId, std::optional<int> hodId) {
    s.require(Action::MANAGE_COURSES);
    if (!courses.findDepartment(departmentId)) throw NotFoundError("No department with that id.");
    if (hodId) checkCanBeHod(*hodId, departmentId);
    courses.setDepartmentHod(departmentId, hodId);
}

std::vector<std::unique_ptr<User>> AdminService::listUsers(Session& s) {
    s.require(Action::MANAGE_USERS);
    return users.listAll();
}

std::vector<Department> AdminService::listDepartments(Session& s) {
    s.require(Action::MANAGE_COURSES);
    return courses.listDepartments();
}

std::vector<Course> AdminService::listCourses(Session& s) {
    s.require(Action::MANAGE_COURSES);
    return courses.listCourses();
}
