// Validation.h - small input checks shared by the services.
#pragma once

#include <string>

#include "Errors.h"

// True if the text is empty or only spaces/tabs/newlines.
inline bool isBlank(const std::string& text) {
    return text.find_first_not_of(" \t\r\n") == std::string::npos;
}

inline void requireNotBlank(const std::string& text, const std::string& fieldName) {
    if (isBlank(text)) throw ValidationError(fieldName + " cannot be empty.");
}
