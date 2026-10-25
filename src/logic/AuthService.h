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

private:
    IUserRepository& users;
    IPasswordHasher& hasher;
};
