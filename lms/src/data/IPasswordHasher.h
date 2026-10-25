// IPasswordHasher.h - hashing and checking passwords (NFR04).
#pragma once

#include <string>

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    virtual std::string hash(const std::string& password) = 0;
    virtual bool verify(const std::string& password, const std::string& hash) = 0;
};
