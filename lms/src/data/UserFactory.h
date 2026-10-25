// UserFactory.h - builds the right User subclass from the role column (Factory pattern).
#pragma once

#include <memory>
#include <string>

#include "Types.h"
#include "User.h"

class UserFactory {
public:
    static std::unique_ptr<User> create(Role role, int id, const std::string& username,
                                        const std::string& fullName, const std::string& hash,
                                        bool active);
};
