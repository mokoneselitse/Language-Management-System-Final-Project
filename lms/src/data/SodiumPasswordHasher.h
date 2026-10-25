// SodiumPasswordHasher.h - Argon2id password hashing with libsodium.
// crypto_pwhash_str() picks a random 16-byte salt and stores the salt and the
// cost settings inside the hash string, so only that one string is saved.
#pragma once

#include <cstddef>
#include <string>

#include "IPasswordHasher.h"

class SodiumPasswordHasher : public IPasswordHasher {
public:
    SodiumPasswordHasher();
    std::string hash(const std::string& password) override;
    bool verify(const std::string& password, const std::string& hash) override;

private:
    unsigned long long opsLimit;
    std::size_t memLimit;
};
