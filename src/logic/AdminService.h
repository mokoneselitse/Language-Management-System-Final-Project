// AdminService.h - administrator functions (FR01 - FR04).
#pragma once

#include <optional>
#include <string>

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

private:
    IUserRepository& users;
    ICourseRepository& courses;
    IPasswordHasher& hasher;
    IClock& clock;
};
