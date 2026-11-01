// Unit tests for the late-penalty strategy (FR18, C2).
// Test design: equivalence partitioning on daysLate
//   {<= 0 on time} {1..3 penalised} {> 3 zero}
// plus boundary values 0, 1, 3, 4 and the worked example from the design document.
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "LatePenalty.h"

using Catch::Approx;

TEST_CASE("UT-01 Standard penalty matches the design worked example (max 50, raw 40)",
          "[unit][penalty][FR18]") {
    StandardLatePenalty penalty;
    CHECK(penalty.apply(40, 50, 0) == Approx(40));  // on time
    CHECK(penalty.apply(40, 50, 1) == Approx(35));  // -5
    CHECK(penalty.apply(40, 50, 2) == Approx(30));  // -10
    CHECK(penalty.apply(40, 50, 3) == Approx(25));  // -15
    CHECK(penalty.apply(40, 50, 4) == Approx(0));   // more than 3 days
}

TEST_CASE("UT-02 Penalty boundaries: 3 days is penalised, 4 days gives zero",
          "[unit][penalty][FR18][boundary]") {
    StandardLatePenalty penalty;
    CHECK(penalty.apply(100, 100, 3) == Approx(70));
    CHECK(penalty.apply(100, 100, 4) == Approx(0));
    CHECK(penalty.apply(100, 100, 30) == Approx(0));
}

TEST_CASE("UT-03 Penalty never makes a mark negative", "[unit][penalty][FR18]") {
    StandardLatePenalty penalty;
    CHECK(penalty.apply(5, 50, 3) == Approx(0));   // 5 - 15 would be -10
    CHECK(penalty.apply(0, 50, 1) == Approx(0));
}

TEST_CASE("UT-04 Penalty is 10% of the MAXIMUM mark, not of the raw mark",
          "[unit][penalty][FR18]") {
    StandardLatePenalty penalty;
    CHECK(penalty.apply(20, 80, 1) == Approx(12));  // 20 - 8, not 20 - 2
}

TEST_CASE("UT-05 Days late of zero or less means no penalty", "[unit][penalty][FR18]") {
    StandardLatePenalty penalty;
    CHECK(penalty.apply(37.5, 50, 0) == Approx(37.5));
    CHECK(penalty.apply(37.5, 50, -1) == Approx(37.5));
}

TEST_CASE("UT-06 NoLatePenalty always returns the raw mark", "[unit][penalty]") {
    NoLatePenalty penalty;
    CHECK(penalty.apply(40, 50, 0) == Approx(40));
    CHECK(penalty.apply(40, 50, 10) == Approx(40));
}
