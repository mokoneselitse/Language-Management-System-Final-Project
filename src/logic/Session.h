// Session.h - the user who is logged in.
// Every service method calls require() first, so a user can only do what
// their role allows (FR21). The check is done by User::canPerform().
#pragma once

#include <memory>

#include "Types.h"
#include "User.h"

class Session {
public:
    explicit Session(std::unique_ptr<User> user);
    const User& user() const;
    void require(Action action) const;  // throws AccessDeniedError

private:
    std::unique_ptr<User> loggedInUser;
};
