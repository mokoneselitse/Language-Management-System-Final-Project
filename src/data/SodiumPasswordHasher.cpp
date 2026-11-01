#include "SodiumPasswordHasher.h"

#include <sodium.h>

#include "Errors.h"

SodiumPasswordHasher::SodiumPasswordHasher()
    : SodiumPasswordHasher(crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) {}

SodiumPasswordHasher::SodiumPasswordHasher(unsigned long long opsLimit, std::size_t memLimit)
    : opsLimit(opsLimit), memLimit(memLimit) {
    if (sodium_init() < 0) {  // must be called once before using libsodium
        throw LmsError("Could not start libsodium.");
    }
}

std::string SodiumPasswordHasher::hash(const std::string& password) {
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(out, password.c_str(), password.size(), opsLimit, memLimit) != 0) {
        throw LmsError("Out of memory while hashing the password.");
    }
    return out;  // looks like $argon2id$v=19$m=...,t=...,p=1$<salt>$<hash>
}

bool SodiumPasswordHasher::verify(const std::string& password, const std::string& hash) {
    return crypto_pwhash_str_verify(hash.c_str(), password.c_str(), password.size()) == 0;
}
