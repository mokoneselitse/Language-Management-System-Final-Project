#include "SqliteRepositories.h"

#include "SqlHelpers.h"

static Course readCourse(Statement& st) {
    return Course(st.columnInt(0), st.columnText(1), st.columnText(2), st.columnInt(3),
                  readOptionalInt(st, 4));
}

int SqliteCourseRepository::addDepartment(const std::string& name, std::optional<int> hodId) {
    Statement st = db.prepare("INSERT INTO department (name, hod_user_id) VALUES (?, ?);");
    st.bind(1, name);
    bindOptional(st, 2, hodId);
    st.step();
    return db.lastInsertId();
}

int SqliteCourseRepository::addCourse(const std::string& code, const std::string& title,
                                      int departmentId) {
    Statement st =
        db.prepare("INSERT INTO course (code, title, department_id) VALUES (?, ?, ?);");
    st.bind(1, code).bind(2, title).bind(3, departmentId);
    st.step();
    return db.lastInsertId();
}

std::optional<Course> SqliteCourseRepository::findCourse(int id) {
    Statement st = db.prepare(
        "SELECT course_id, code, title, department_id, lecturer_id FROM course "
        "WHERE course_id = ?;");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readCourse(st);
}

void SqliteCourseRepository::assignLecturer(int courseId, int lecturerId) {
    Statement st = db.prepare("UPDATE course SET lecturer_id = ? WHERE course_id = ?;");
    st.bind(1, lecturerId).bind(2, courseId);
    st.step();
}

void SqliteCourseRepository::enrol(int studentId, int courseId, Timestamp at) {
    Statement st = db.prepare(
        "INSERT INTO enrolment (student_id, course_id, enrolled_at) VALUES (?, ?, ?);");
    st.bind(1, studentId).bind(2, courseId).bind(3, toUnixSeconds(at));
    st.step();
}

bool SqliteCourseRepository::isEnrolled(int studentId, int courseId) {
    Statement st =
        db.prepare("SELECT 1 FROM enrolment WHERE student_id = ? AND course_id = ?;");
    st.bind(1, studentId).bind(2, courseId);
    return st.step();
}

std::vector<Course> SqliteCourseRepository::coursesForStudent(int studentId) {
    Statement st = db.prepare(
        "SELECT c.course_id, c.code, c.title, c.department_id, c.lecturer_id "
        "FROM course c JOIN enrolment e ON e.course_id = c.course_id "
        "WHERE e.student_id = ? ORDER BY c.code;");
    st.bind(1, studentId);
    std::vector<Course> courses;
    while (st.step()) courses.push_back(readCourse(st));
    return courses;
}

std::optional<int> SqliteCourseRepository::departmentOfHod(int hodId) {
    Statement st = db.prepare("SELECT department_id FROM department WHERE hod_user_id = ?;");
    st.bind(1, hodId);
    if (!st.step()) return std::nullopt;
    return st.columnInt(0);
}

void SqliteCourseRepository::updateCourse(int id, const std::string& code,
                                          const std::string& title) {
    Statement st = db.prepare("UPDATE course SET code = ?, title = ? WHERE course_id = ?;");
    st.bind(1, code).bind(2, title).bind(3, id);
    st.step();
}

void SqliteCourseRepository::setDepartmentHod(int departmentId, std::optional<int> hodId) {
    Statement st = db.prepare("UPDATE department SET hod_user_id = ? WHERE department_id = ?;");
    bindOptional(st, 1, hodId);
    st.bind(2, departmentId);
    st.step();
}

std::optional<Department> SqliteCourseRepository::findDepartment(int id) {
    Statement st = db.prepare(
        "SELECT department_id, name, hod_user_id FROM department WHERE department_id = ?;");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return Department(st.columnInt(0), st.columnText(1), readOptionalInt(st, 2));
}

std::vector<Department> SqliteCourseRepository::listDepartments() {
    Statement st =
        db.prepare("SELECT department_id, name, hod_user_id FROM department ORDER BY name;");
    std::vector<Department> list;
    while (st.step()) {
        list.emplace_back(st.columnInt(0), st.columnText(1), readOptionalInt(st, 2));
    }
    return list;
}

std::vector<Course> SqliteCourseRepository::listCourses() {
    Statement st = db.prepare(
        "SELECT course_id, code, title, department_id, lecturer_id FROM course ORDER BY code;");
    std::vector<Course> list;
    while (st.step()) list.push_back(readCourse(st));
    return list;
}

std::vector<Course> SqliteCourseRepository::coursesForLecturer(int lecturerId) {
    Statement st = db.prepare(
        "SELECT course_id, code, title, department_id, lecturer_id FROM course "
        "WHERE lecturer_id = ? ORDER BY code;");
    st.bind(1, lecturerId);
    std::vector<Course> list;
    while (st.step()) list.push_back(readCourse(st));
    return list;
}
