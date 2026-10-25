#include "SqliteRepositories.h"

#include "SqlHelpers.h"

static const char* SELECT_ASSIGNMENT =
    "SELECT a.assignment_id, a.course_id, a.title, a.description, a.deadline, a.max_mark, "
    "       a.created_by "
    "FROM assignment a ";

static Assignment readAssignment(Statement& st) {
    return Assignment(st.columnInt(0), st.columnInt(1), st.columnText(2), st.columnText(3),
                      readTime(st, 4), st.columnDouble(5), st.columnInt(6));
}

static Extension readExtension(Statement& st) {
    return Extension(st.columnInt(0), st.columnInt(1), st.columnInt(2), readTime(st, 3),
                     st.columnText(4), st.columnInt(5), readTime(st, 6));
}

int SqliteAssignmentRepository::add(const Assignment& a) {
    Statement st = db.prepare(
        "INSERT INTO assignment (course_id, title, description, deadline, max_mark, created_by, "
        "created_at) VALUES (?, ?, ?, ?, ?, ?, ?);");
    st.bind(1, a.getCourseId())
        .bind(2, a.getTitle())
        .bind(3, a.getDescription())
        .bind(4, toUnixSeconds(a.getDeadline()))
        .bind(5, a.getMaxMark())
        .bind(6, a.getCreatedBy())
        .bind(7, nowUnix());
    st.step();
    return db.lastInsertId();
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int id) {
    Statement st = db.prepare(std::string(SELECT_ASSIGNMENT) + "WHERE a.assignment_id = ?;");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readAssignment(st);
}

// Assignments of every course the student is enrolled on.
std::vector<Assignment> SqliteAssignmentRepository::forStudent(int studentId) {
    Statement st = db.prepare(std::string(SELECT_ASSIGNMENT) +
                              "JOIN enrolment e ON e.course_id = a.course_id "
                              "WHERE e.student_id = ? ORDER BY a.deadline;");
    st.bind(1, studentId);
    std::vector<Assignment> list;
    while (st.step()) list.push_back(readAssignment(st));
    return list;
}

// Assignments of every course the lecturer is assigned to.
std::vector<Assignment> SqliteAssignmentRepository::forLecturer(int lecturerId) {
    Statement st = db.prepare(std::string(SELECT_ASSIGNMENT) +
                              "JOIN course c ON c.course_id = a.course_id "
                              "WHERE c.lecturer_id = ? ORDER BY a.deadline;");
    st.bind(1, lecturerId);
    std::vector<Assignment> list;
    while (st.step()) list.push_back(readAssignment(st));
    return list;
}

// One extension per student per assignment: granting again replaces the old one.
void SqliteAssignmentRepository::saveExtension(const Extension& e) {
    Statement st = db.prepare(
        "INSERT INTO extension (assignment_id, student_id, new_deadline, reason, granted_by, "
        "granted_at) VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT (assignment_id, student_id) DO UPDATE SET "
        "new_deadline = excluded.new_deadline, reason = excluded.reason, "
        "granted_by = excluded.granted_by, granted_at = excluded.granted_at;");
    st.bind(1, e.getAssignmentId())
        .bind(2, e.getStudentId())
        .bind(3, toUnixSeconds(e.getNewDeadline()))
        .bind(4, e.getReason())
        .bind(5, e.getGrantedBy())
        .bind(6, toUnixSeconds(e.getGrantedAt()));
    st.step();
}

std::optional<Extension> SqliteAssignmentRepository::findExtension(int assignmentId,
                                                                   int studentId) {
    Statement st = db.prepare(
        "SELECT extension_id, assignment_id, student_id, new_deadline, reason, granted_by, "
        "granted_at FROM extension WHERE assignment_id = ? AND student_id = ?;");
    st.bind(1, assignmentId).bind(2, studentId);
    if (!st.step()) return std::nullopt;
    return readExtension(st);
}
