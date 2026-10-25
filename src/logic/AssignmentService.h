// AssignmentService.h - assignments and extensions (FR05, FR06, FR14, FR15, FR23).
#pragma once

#include <string>
#include <vector>

#include "IClock.h"
#include "Repositories.h"
#include "Session.h"

class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      IClock& clock);

    int createAssignment(Session& s, int courseId, const std::string& title,
                         const std::string& desc, Timestamp deadline, double maxMark);
    void grantExtension(Session& s, int assignmentId, int studentId, Timestamp newDeadline,
                        const std::string& reason);
    std::vector<Assignment> listForStudent(Session& s);
    // byDeadline = true: sorted by deadline; false: in the order they were created.
    std::vector<Assignment> listForLecturer(Session& s, bool byDeadline);
    std::vector<Course> coursesForStudent(Session& s);

private:
    IAssignmentRepository& assignments;
    ICourseRepository& courses;
    IClock& clock;
};
