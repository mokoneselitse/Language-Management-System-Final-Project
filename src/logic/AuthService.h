// AuthService.h - logging in (FR20).
#pragma once

#include <string>

#include "IPasswordHasher.h"
#include "Repositories.h"
#include "Session.h"

class AuthService {
public:
    AuthService(IUserRepository& users, IPasswordHasher& hasher);
    Session login(const std::string& username, const std::string& password);
    // Added during coding: full name of a user, so menus can show names instead of ids.
    std::string nameOf(Session& s, int userId);

private:
    IUserRepository& users;
    IPasswordHasher& hasher;
};
