// Enrolment.h - association class between Student and Course.
#pragma once

#include "Types.h"

class Enrolment {
public:
    Enrolment(int studentId, int courseId, Timestamp enrolledAt)
        : studentId(studentId), courseId(courseId), enrolledAt(enrolledAt) {}

    int getStudentId() const { return studentId; }
    int getCourseId() const { return courseId; }
    Timestamp getEnrolledAt() const { return enrolledAt; }

private:
    int studentId;
    int courseId;
    Timestamp enrolledAt;
};
