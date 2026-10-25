#include "AuthService.h"

#include "Errors.h"

AuthService::AuthService(IUserRepository& users, IPasswordHasher& hasher)
    : users(users), hasher(hasher) {}

Session AuthService::login(const std::string& username, const std::string& password) {
    std::unique_ptr<User> user = users.findByUsername(username);
    // Same message for a wrong username and a wrong password, so nobody can
    // find out which usernames exist.
    if (user == nullptr || !hasher.verify(password, user->getPasswordHash())) {
        throw AuthenticationError("Invalid username or password.");
    }
    if (!user->isActive()) {
        throw AuthenticationError("This account has been deactivated.");
    }
    return Session(std::move(user));
}
