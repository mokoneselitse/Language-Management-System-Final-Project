// Unit tests for SubmissionService::daysLate (state table: ceil((t - deadline) / 24 h)).
// Boundary value analysis around the deadline and around each whole day.
#include <catch2/catch_test_macros.hpp>

#include "SubmissionService.h"
#include "TestHelpers.h"

TEST_CASE("UT-07 Submitting before or exactly at the deadline is 0 days late",
          "[unit][deadline][FR16][boundary]") {
    Timestamp d = testDeadline();
    CHECK(SubmissionService::daysLate(d, d - hours(5)) == 0);
    CHECK(SubmissionService::daysLate(d, d - seconds(1)) == 0);
    CHECK(SubmissionService::daysLate(d, d) == 0);  // t = deadline is on time
}

TEST_CASE("UT-08 One second late counts as one whole day", "[unit][deadline][FR18][boundary]") {
    Timestamp d = testDeadline();
    CHECK(SubmissionService::daysLate(d, d + seconds(1)) == 1);
}

TEST_CASE("UT-09 Day boundaries are rounded up", "[unit][deadline][FR18][boundary]") {
    Timestamp d = testDeadline();
    CHECK(SubmissionService::daysLate(d, d + hours(24)) == 1);
    CHECK(SubmissionService::daysLate(d, d + hours(24) + seconds(1)) == 2);
    CHECK(SubmissionService::daysLate(d, d + hours(72)) == 3);
    CHECK(SubmissionService::daysLate(d, d + hours(72) + seconds(1)) == 4);  // now gets 0 marks
}
