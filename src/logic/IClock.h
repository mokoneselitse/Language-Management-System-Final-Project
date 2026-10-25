// IClock.h - where the services get the current time from.
// main() passes a SystemClock; tests can pass a fixed clock instead, so
// deadline checks give the same result every time they run.
#pragma once

#include <chrono>

#include "Types.h"

class IClock {
public:
    virtual ~IClock() = default;
    virtual Timestamp now() = 0;
};

class SystemClock : public IClock {
public:
    Timestamp now() override { return std::chrono::system_clock::now(); }
};
