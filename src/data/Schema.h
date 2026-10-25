// Schema.h - the SQL schema from the design document (Database Design section).
// "IF NOT EXISTS" is added so the schema can be run every time the program
// starts: the first start creates lms.db, later starts leave it unchanged.
#pragma once

inline const char* SCHEMA_SQL = R"SQL(
CREATE TABLE IF NOT EXISTS users (
    user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    username      TEXT NOT NULL UNIQUE CHECK (length(username) BETWEEN 3 AND 32),
    password_hash TEXT NOT NULL CHECK (password_hash LIKE '$argon2id$%'),
    full_name     TEXT NOT NULL CHECK (length(full_name) > 0),
    role          TEXT NOT NULL CHECK (role IN ('STUDENT','LECTURER','HOD','ADMIN')),
    is_active     INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
    created_at    INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS department (
    department_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name          TEXT NOT NULL UNIQUE CHECK (length(name) > 0),
    hod_user_id   INTEGER UNIQUE REFERENCES users(user_id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS course (
    course_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    code          TEXT NOT NULL UNIQUE CHECK (length(code) > 0),
    title         TEXT NOT NULL CHECK (length(title) > 0),
    department_id INTEGER NOT NULL REFERENCES department(department_id) ON DELETE RESTRICT,
    lecturer_id   INTEGER REFERENCES users(user_id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS enrolment (
    student_id  INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    course_id   INTEGER NOT NULL REFERENCES course(course_id) ON DELETE RESTRICT,
    enrolled_at INTEGER NOT NULL,
    PRIMARY KEY (student_id, course_id)
);

CREATE TABLE IF NOT EXISTS assignment (
    assignment_id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id     INTEGER NOT NULL REFERENCES course(course_id) ON DELETE RESTRICT,
    title         TEXT NOT NULL CHECK (length(title) > 0),
    description   TEXT NOT NULL DEFAULT '',
    deadline      INTEGER NOT NULL,
    max_mark      REAL NOT NULL CHECK (max_mark > 0),
    created_by    INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    created_at    INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS extension (
    extension_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id INTEGER NOT NULL REFERENCES assignment(assignment_id) ON DELETE RESTRICT,
    student_id    INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    new_deadline  INTEGER NOT NULL,
    reason        TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    granted_by    INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    granted_at    INTEGER NOT NULL,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE IF NOT EXISTS submission (
    submission_id INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id INTEGER NOT NULL REFERENCES assignment(assignment_id) ON DELETE RESTRICT,
    student_id    INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    file_path     TEXT NOT NULL CHECK (length(file_path) > 0),
    submitted_at  INTEGER NOT NULL,
    attempt_no    INTEGER NOT NULL DEFAULT 1 CHECK (attempt_no >= 1),
    status        TEXT NOT NULL CHECK (status IN ('SUBMITTED','LATE','MARKED','MODERATED','RELEASED')),
    days_late     INTEGER NOT NULL DEFAULT 0 CHECK (days_late >= 0),
    raw_mark      REAL CHECK (raw_mark IS NULL OR raw_mark >= 0),
    mark          REAL CHECK (mark IS NULL OR mark >= 0),
    feedback      TEXT,
    marked_by     INTEGER REFERENCES users(user_id) ON DELETE RESTRICT,
    marked_at     INTEGER,
    moderated_by  INTEGER REFERENCES users(user_id) ON DELETE RESTRICT,
    moderated_at  INTEGER,
    released_at   INTEGER,
    UNIQUE (assignment_id, student_id),
    CHECK ((status = 'LATE') = (days_late > 0) OR status IN ('MARKED','MODERATED','RELEASED')),
    CHECK (status IN ('SUBMITTED','LATE')
           OR (raw_mark IS NOT NULL AND mark IS NOT NULL AND marked_by IS NOT NULL AND marked_at IS NOT NULL)),
    CHECK (status IN ('MODERATED','RELEASED') OR (moderated_by IS NULL AND moderated_at IS NULL)),
    CHECK (status = 'RELEASED' OR released_at IS NULL),
    CHECK (status <> 'RELEASED' OR released_at IS NOT NULL)
);

CREATE TABLE IF NOT EXISTS audit_log (
    audit_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id INTEGER NOT NULL REFERENCES submission(submission_id) ON DELETE RESTRICT,
    changed_by    INTEGER NOT NULL REFERENCES users(user_id) ON DELETE RESTRICT,
    changed_at    INTEGER NOT NULL,
    action        TEXT NOT NULL CHECK (action IN ('MARK','REMARK','MODERATE')),
    old_mark      REAL,
    new_mark      REAL NOT NULL CHECK (new_mark >= 0),
    reason        TEXT NOT NULL CHECK (length(trim(reason)) > 0)
);

CREATE TRIGGER IF NOT EXISTS audit_log_no_update BEFORE UPDATE ON audit_log
BEGIN SELECT RAISE(ABORT, 'audit_log is append-only'); END;

CREATE TRIGGER IF NOT EXISTS audit_log_no_delete BEFORE DELETE ON audit_log
BEGIN SELECT RAISE(ABORT, 'audit_log is append-only'); END;

CREATE INDEX IF NOT EXISTS idx_submission_status  ON submission(status);
CREATE INDEX IF NOT EXISTS idx_submission_student ON submission(student_id);
CREATE INDEX IF NOT EXISTS idx_assignment_course  ON assignment(course_id);
CREATE INDEX IF NOT EXISTS idx_course_department  ON course(department_id);
CREATE INDEX IF NOT EXISTS idx_audit_submission   ON audit_log(submission_id);
CREATE INDEX IF NOT EXISTS idx_audit_changed_at   ON audit_log(changed_at);

CREATE VIEW IF NOT EXISTS student_result_view AS
SELECT s.submission_id,
       s.student_id,
       a.assignment_id,
       a.title AS assignment_title,
       a.max_mark,
       s.status,
       s.days_late,
       CASE WHEN s.status IN ('MARKED','MODERATED','RELEASED') THEN s.feedback END AS feedback,
       CASE WHEN s.status = 'RELEASED' THEN s.mark END AS mark
FROM submission s
JOIN assignment a ON a.assignment_id = s.assignment_id;
)SQL";
