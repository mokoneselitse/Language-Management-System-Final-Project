#include "SqliteRepositories.h"

#include "SqlHelpers.h"

static const char* SELECT_SUBMISSION =
    "SELECT s.submission_id, s.assignment_id, s.student_id, s.file_path, s.submitted_at, "
    "       s.attempt_no, s.status, s.days_late, s.raw_mark, s.mark, s.feedback, "
    "       s.marked_by, s.marked_at, s.moderated_by, s.moderated_at, s.released_at "
    "FROM submission s ";

static Submission readSubmission(Statement& st) {
    return Submission(st.columnInt(0), st.columnInt(1), st.columnInt(2), st.columnText(3),
                      readTime(st, 4), st.columnInt(5), statusFromString(st.columnText(6)),
                      st.columnInt(7), readOptionalDouble(st, 8), readOptionalDouble(st, 9),
                      st.columnText(10), readOptionalInt(st, 11), readOptionalTime(st, 12),
                      readOptionalInt(st, 13), readOptionalTime(st, 14),
                      readOptionalTime(st, 15));
}

// Binds every column except submission_id, starting at placeholder 1.
// Used by both insert() and update() so they always match.
static void bindSubmission(Statement& st, const Submission& s) {
    st.bind(1, s.getAssignmentId())
        .bind(2, s.getStudentId())
        .bind(3, s.getFilePath())
        .bind(4, toUnixSeconds(s.getSubmittedAt()))
        .bind(5, s.getAttemptNo())
        .bind(6, toString(s.getStatus()))
        .bind(7, s.getDaysLate());
    bindOptional(st, 8, s.getRawMark());
    bindOptional(st, 9, s.getMark());
    if (s.getFeedback().empty()) st.bindNull(10); else st.bind(10, s.getFeedback());
    bindOptional(st, 11, s.getMarkedBy());
    bindOptional(st, 12, s.getMarkedAt());
    bindOptional(st, 13, s.getModeratedBy());
    bindOptional(st, 14, s.getModeratedAt());
    bindOptional(st, 15, s.getReleasedAt());
}

std::optional<Submission> SqliteSubmissionRepository::find(int assignmentId, int studentId) {
    Statement st = db.prepare(std::string(SELECT_SUBMISSION) +
                              "WHERE s.assignment_id = ? AND s.student_id = ?;");
    st.bind(1, assignmentId).bind(2, studentId);
    if (!st.step()) return std::nullopt;
    return readSubmission(st);
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) {
    Statement st = db.prepare(std::string(SELECT_SUBMISSION) + "WHERE s.submission_id = ?;");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readSubmission(st);
}

int SqliteSubmissionRepository::insert(const Submission& s) {
    Statement st = db.prepare(
        "INSERT INTO submission (assignment_id, student_id, file_path, submitted_at, attempt_no, "
        "status, days_late, raw_mark, mark, feedback, marked_by, marked_at, moderated_by, "
        "moderated_at, released_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    bindSubmission(st, s);
    st.step();
    return db.lastInsertId();
}

void SqliteSubmissionRepository::update(const Submission& s) {
    Statement st = db.prepare(
        "UPDATE submission SET assignment_id = ?, student_id = ?, file_path = ?, "
        "submitted_at = ?, attempt_no = ?, status = ?, days_late = ?, raw_mark = ?, mark = ?, "
        "feedback = ?, marked_by = ?, marked_at = ?, moderated_by = ?, moderated_at = ?, "
        "released_at = ? WHERE submission_id = ?;");
    bindSubmission(st, s);
    st.bind(16, s.getId());
    st.step();
}

// Used by the HoD: submissions in a given state for all courses of a department.
std::vector<Submission> SqliteSubmissionRepository::findByDepartmentAndStatus(
    int departmentId, SubmissionStatus status) {
    Statement st = db.prepare(std::string(SELECT_SUBMISSION) +
                              "JOIN assignment a ON a.assignment_id = s.assignment_id "
                              "JOIN course c ON c.course_id = a.course_id "
                              "WHERE c.department_id = ? AND s.status = ? "
                              "ORDER BY s.submission_id;");
    st.bind(1, departmentId).bind(2, toString(status));
    std::vector<Submission> list;
    while (st.step()) list.push_back(readSubmission(st));
    return list;
}

std::vector<Submission> SqliteSubmissionRepository::findByAssignmentAndStatus(
    int assignmentId, SubmissionStatus status) {
    Statement st = db.prepare(std::string(SELECT_SUBMISSION) +
                              "WHERE s.assignment_id = ? AND s.status = ? "
                              "ORDER BY s.submission_id;");
    st.bind(1, assignmentId).bind(2, toString(status));
    std::vector<Submission> list;
    while (st.step()) list.push_back(readSubmission(st));
    return list;
}

// Students only ever read their results through student_result_view (FR13).
std::vector<StudentResult> SqliteSubmissionRepository::resultsForStudent(int studentId) {
    Statement st = db.prepare(
        "SELECT assignment_title, max_mark, status, days_late, feedback, mark "
        "FROM student_result_view WHERE student_id = ? ORDER BY assignment_id;");
    st.bind(1, studentId);
    std::vector<StudentResult> results;
    while (st.step()) {
        StudentResult r;
        r.assignmentTitle = st.columnText(0);
        r.maxMark = st.columnDouble(1);
        r.status = statusFromString(st.columnText(2));
        r.daysLate = st.columnInt(3);
        if (!st.isNull(4)) r.feedback = st.columnText(4);
        r.mark = readOptionalDouble(st, 5);
        results.push_back(r);
    }
    return results;
}
