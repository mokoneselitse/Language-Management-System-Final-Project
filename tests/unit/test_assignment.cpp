// Unit tests for Assignment::effectiveDeadlineFor (FR06).
#include <catch2/catch_test_macros.hpp>

#include "Assignment.h"
#include "Extension.h"
#include "TestHelpers.h"

TEST_CASE("UT-22 Effective deadline uses a later extension, otherwise the original",
          "[unit][deadline][FR06]") {
    Timestamp d = testDeadline();
    Assignment a(1, 1, "Project", "", d, 50, 2);

    CHECK(a.effectiveDeadlineFor(nullptr) == d);  // no extension

    Extension later(1, 1, 5, d + hours(48), "sick", 2, d);
    CHECK(a.effectiveDeadlineFor(&later) == d + hours(48));

    Extension earlier(2, 1, 5, d - hours(1), "mistake", 2, d);
    CHECK(a.effectiveDeadlineFor(&earlier) == d);  // an earlier date never shortens it
}
