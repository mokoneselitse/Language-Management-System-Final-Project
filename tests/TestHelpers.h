// TestHelpers.h - small helpers shared by the test files.
#pragma once

#include <chrono>

#include "IClock.h"
#include "Types.h"

// A clock that returns whatever time the test sets, so deadline tests give
// the same result every time they run (see the IClock note in the class diagram).
class FixedClock : public IClock {
public:
    explicit FixedClock(Timestamp start) : current(start) {}
    Timestamp now() override { return current; }
    void set(Timestamp t) { current = t; }

private:
    Timestamp current;
};

// A fixed point in time used as "the deadline" in many tests.
inline Timestamp testDeadline() { return fromUnixSeconds(1800000000); }  // 15 Jan 2027

using std::chrono::hours;
using std::chrono::seconds;
