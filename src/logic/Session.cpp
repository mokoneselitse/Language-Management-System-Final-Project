#include "Session.h"

#include <utility>

#include "Errors.h"

Session::Session(std::unique_ptr<User> user) : loggedInUser(std::move(user)) {}

const User& Session::user() const { return *loggedInUser; }

void Session::require(Action action) const {
    if (!loggedInUser->canPerform(action)) {
        throw AccessDeniedError("Your role is not allowed to do this.");
    }
}
