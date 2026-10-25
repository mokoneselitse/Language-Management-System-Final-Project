#include "UserFactory.h"

std::unique_ptr<User> UserFactory::create(Role role, int id, const std::string& username,
                                          const std::string& fullName, const std::string& hash,
                                          bool active) {
    switch (role) {
        case Role::STUDENT:
            return std::make_unique<Student>(id, username, fullName, hash, active);
        case Role::LECTURER:
            return std::make_unique<Lecturer>(id, username, fullName, hash, active);
        case Role::HOD:
            return std::make_unique<HeadOfDepartment>(id, username, fullName, hash, active);
        case Role::ADMIN:
            return std::make_unique<Administrator>(id, username, fullName, hash, active);
    }
    return nullptr;  // not reached: every role is handled above
}
