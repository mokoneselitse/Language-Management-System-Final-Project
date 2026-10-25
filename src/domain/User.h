// User.h - abstract User and the four role classes (domain class diagram).
// Each role overrides canPerform(), so permissions are decided by
// polymorphism instead of a switch on the role.
#pragma once

#include <optional>
#include <string>

#include "Types.h"

class User {
public:
    User(int id, std::string username, std::string fullName, std::string passwordHash,
         bool active);
    virtual ~User() = default;

    int getId() const;
    std::string getUsername() const;
    std::string getFullName() const;
    std::string getPasswordHash() const;
    bool isActive() const;

    virtual Role getRole() const = 0;
    virtual bool canPerform(Action action) const = 0;

protected:
    int id;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    bool active;
};

class Administrator : public User {
public:
    using User::User;
    Role getRole() const override;
    bool canPerform(Action action) const override;
};

class Lecturer : public User {
public:
    using User::User;
    Role getRole() const override;
    bool canPerform(Action action) const override;
};

class Student : public User {
public:
    using User::User;
    Role getRole() const override;
    bool canPerform(Action action) const override;
};

class HeadOfDepartment : public User {
public:
    using User::User;
    Role getRole() const override;
    bool canPerform(Action action) const override;
    std::optional<int> getDepartmentId() const;
    // Filled in by the user repository from department.hod_user_id.
    void setDepartmentId(std::optional<int> id);

private:
    std::optional<int> departmentId;
};
