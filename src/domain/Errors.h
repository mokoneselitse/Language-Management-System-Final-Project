// Errors.h - exception hierarchy from the business-logic class diagram.
// The console menus catch LmsError, print the message and carry on,
// so a bad input or a database problem never crashes the program.
#pragma once

#include <stdexcept>
#include <string>

class LmsError : public std::runtime_error {
public:
    explicit LmsError(const std::string& message) : std::runtime_error(message) {}
};

// Wrong username or password, or a deactivated account.
class AuthenticationError : public LmsError {
public:
    explicit AuthenticationError(const std::string& message) : LmsError(message) {}
};

// The user's role (or ownership) does not allow the action.
class AccessDeniedError : public LmsError {
public:
    explicit AccessDeniedError(const std::string& message) : LmsError(message) {}
};

// Bad input, e.g. a mark above the maximum or an empty reason.
class ValidationError : public LmsError {
public:
    explicit ValidationError(const std::string& message) : LmsError(message) {}
};

// A submission was asked to move to a state the state machine does not allow.
class InvalidTransitionError : public LmsError {
public:
    explicit InvalidTransitionError(const std::string& message) : LmsError(message) {}
};

// A record (user, course, assignment, submission ...) does not exist.
class NotFoundError : public LmsError {
public:
    explicit NotFoundError(const std::string& message) : LmsError(message) {}
};

// Anything that goes wrong inside SQLite.
class DatabaseError : public LmsError {
public:
    explicit DatabaseError(const std::string& message) : LmsError(message) {}
};
