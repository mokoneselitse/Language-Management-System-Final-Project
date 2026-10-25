// Repositories.h - the repository interfaces from the data access class diagram.
// The services only know these interfaces; the Sqlite... classes implement them.
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Assignment.h"
#include "AuditEntry.h"
#include "Course.h"
#include "Department.h"
#include "Extension.h"
#include "StudentResult.h"
#include "Submission.h"
#include "Types.h"
#include "User.h"

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual std::unique_ptr<User> findById(int id) = 0;
    virtual int add(Role role, const std::string& username, const std::string& fullName,
                    const std::string& hash) = 0;
    virtual void setActive(int id, bool active) = 0;
    // Added during coding (FR01 edit, admin user list):
    virtual void update(int id, const std::string& username, const std::string& fullName) = 0;
    virtual void setPasswordHash(int id, const std::string& hash) = 0;
    virtual std::vector<std::unique_ptr<User>> listAll() = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual int addDepartment(const std::string& name, std::optional<int> hodId) = 0;
    virtual int addCourse(const std::string& code, const std::string& title, int departmentId) = 0;
    virtual std::optional<Course> findCourse(int id) = 0;
    virtual void assignLecturer(int courseId, int lecturerId) = 0;
    virtual void enrol(int studentId, int courseId, Timestamp at) = 0;
    virtual bool isEnrolled(int studentId, int courseId) = 0;
    virtual std::vector<Course> coursesForStudent(int studentId) = 0;
    virtual std::optional<int> departmentOfHod(int hodId) = 0;
    // Added during coding (FR02 manage, lecturer course list):
    virtual void updateCourse(int id, const std::string& code, const std::string& title) = 0;
    virtual void setDepartmentHod(int departmentId, std::optional<int> hodId) = 0;
    virtual std::optional<Department> findDepartment(int id) = 0;
    virtual std::vector<Department> listDepartments() = 0;
    virtual std::vector<Course> listCourses() = 0;
    virtual std::vector<Course> coursesForLecturer(int lecturerId) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int add(const Assignment& a) = 0;
    virtual std::optional<Assignment> findById(int id) = 0;
    virtual std::vector<Assignment> forStudent(int studentId) = 0;
    virtual std::vector<Assignment> forLecturer(int lecturerId) = 0;
    virtual void saveExtension(const Extension& e) = 0;
    virtual std::optional<Extension> findExtension(int assignmentId, int studentId) = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual std::optional<Submission> find(int assignmentId, int studentId) = 0;
    virtual std::optional<Submission> findById(int id) = 0;
    virtual int insert(const Submission& s) = 0;
    virtual void update(const Submission& s) = 0;
    virtual std::vector<Submission> findByDepartmentAndStatus(int departmentId,
                                                              SubmissionStatus st) = 0;
    virtual std::vector<Submission> findByAssignmentAndStatus(int assignmentId,
                                                              SubmissionStatus st) = 0;
    virtual std::vector<StudentResult> resultsForStudent(int studentId) = 0;
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual void append(const AuditEntry& e) = 0;
    virtual std::vector<AuditEntry> forSubmission(int submissionId) = 0;
};
