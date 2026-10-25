// StudentResult.h - one row of student_result_view (what a student sees).
// The view leaves mark empty until the status is RELEASED (FR13).
#pragma once

#include <optional>
#include <string>

#include "Types.h"

struct StudentResult {
    std::string assignmentTitle;
    double maxMark = 0;
    SubmissionStatus status = SubmissionStatus::SUBMITTED;
    int daysLate = 0;
    std::optional<std::string> feedback;
    std::optional<double> mark;
};
