// AdminService.h - administrator functions (FR01 - FR04).
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "IClock.h"
#include "IPasswordHasher.h"
#include "Repositories.h"
#include "Session.h"

class AdminService {
public:
    AdminService(IUserRepository& users, ICourseRepository& courses, IPasswordHasher& hasher,
                 IClock& clock);

    int createUser(Session& s, Role role, const std::string& username, const std::string& name,
                   const std::string& pw);
    void setUserActive(Session& s, int userId, bool active);
    int createDepartment(Session& s, const std::string& name, std::optional<int> hodId);
    int createCourse(Session& s, const std::string& code, const std::string& title,
                     int departmentId);
    void assignLecturer(Session& s, int courseId, int lecturerId);
    void enrolStudent(Session& s, int studentId, int courseId);

    // Added during coding: edit users (FR01) and manage departments/courses (FR02).
    void editUser(Session& s, int userId, const std::string& username, const std::string& name);
    void resetPassword(Session& s, int userId, const std::string& newPassword);
    void editCourse(Session& s, int courseId, const std::string& code, const std::string& title);
    void setDepartmentHod(Session& s, int departmentId, std::optional<int> hodId);
    std::vector<std::unique_ptr<User>> listUsers(Session& s);
    std::vector<Department> listDepartments(Session& s);
    std::vector<Course> listCourses(Session& s);

private:
    static void checkUsername(const std::string& username);
    void checkCanBeHod(int hodId, int allowedDepartmentId);

    IUserRepository& users;
    ICourseRepository& courses;
    IPasswordHasher& hasher;
    IClock& clock;
};
