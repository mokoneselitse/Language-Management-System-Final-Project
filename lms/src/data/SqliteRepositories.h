// SqliteRepositories.h - SQLite versions of the five repositories.
// Each one only holds a reference to the shared SqliteDatabase.
#pragma once

#include "Repositories.h"
#include "SqliteDatabase.h"

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(SqliteDatabase& db) : db(db) {}
    std::unique_ptr<User> findByUsername(const std::string& username) override;
    std::unique_ptr<User> findById(int id) override;
    int add(Role role, const std::string& username, const std::string& fullName,
            const std::string& hash) override;
    void setActive(int id, bool active) override;

private:
    SqliteDatabase& db;
};

class SqliteCourseRepository : public ICourseRepository {
public:
    explicit SqliteCourseRepository(SqliteDatabase& db) : db(db) {}
    int addDepartment(const std::string& name, std::optional<int> hodId) override;
    int addCourse(const std::string& code, const std::string& title, int departmentId) override;
    std::optional<Course> findCourse(int id) override;
    void assignLecturer(int courseId, int lecturerId) override;
    void enrol(int studentId, int courseId, Timestamp at) override;
    bool isEnrolled(int studentId, int courseId) override;
    std::vector<Course> coursesForStudent(int studentId) override;
    std::optional<int> departmentOfHod(int hodId) override;

private:
    SqliteDatabase& db;
};

class SqliteAssignmentRepository : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(SqliteDatabase& db) : db(db) {}
    int add(const Assignment& a) override;
    std::optional<Assignment> findById(int id) override;
    std::vector<Assignment> forStudent(int studentId) override;
    std::vector<Assignment> forLecturer(int lecturerId) override;
    void saveExtension(const Extension& e) override;
    std::optional<Extension> findExtension(int assignmentId, int studentId) override;

private:
    SqliteDatabase& db;
};

class SqliteSubmissionRepository : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(SqliteDatabase& db) : db(db) {}
    std::optional<Submission> find(int assignmentId, int studentId) override;
    std::optional<Submission> findById(int id) override;
    int insert(const Submission& s) override;
    void update(const Submission& s) override;
    std::vector<Submission> findByDepartmentAndStatus(int departmentId,
                                                      SubmissionStatus st) override;
    std::vector<Submission> findByAssignmentAndStatus(int assignmentId,
                                                      SubmissionStatus st) override;
    std::vector<StudentResult> resultsForStudent(int studentId) override;

private:
    SqliteDatabase& db;
};

class SqliteAuditRepository : public IAuditRepository {
public:
    explicit SqliteAuditRepository(SqliteDatabase& db) : db(db) {}
    void append(const AuditEntry& e) override;
    std::vector<AuditEntry> forSubmission(int submissionId) override;

private:
    SqliteDatabase& db;
};
