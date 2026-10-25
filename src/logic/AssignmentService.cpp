#include "AssignmentService.h"

#include <algorithm>

#include "Errors.h"
#include "Validation.h"

AssignmentService::AssignmentService(IAssignmentRepository& assignments,
                                     ICourseRepository& courses, IClock& clock)
    : assignments(assignments), courses(courses), clock(clock) {}

int AssignmentService::createAssignment(Session& s, int courseId, const std::string& title,
                                        const std::string& desc, Timestamp deadline,
                                        double maxMark) {
    s.require(Action::CREATE_ASSIGNMENT);
    std::optional<Course> course = courses.findCourse(courseId);
    if (!course) throw NotFoundError("No course with that id.");
    // FR05: only for a course the lecturer is assigned to.
    if (course->getLecturerId() != s.user().getId()) {
        throw AccessDeniedError("You are not the lecturer of this course.");
    }
    requireNotBlank(title, "Title");
    if (maxMark <= 0) throw ValidationError("Maximum mark must be greater than 0.");
    if (deadline <= clock.now()) throw ValidationError("The deadline must be in the future.");

    return assignments.add(
        Assignment(0, courseId, title, desc, deadline, maxMark, s.user().getId()));
}

void AssignmentService::grantExtension(Session& s, int assignmentId, int studentId,
                                       Timestamp newDeadline, const std::string& reason) {
    s.require(Action::GRANT_EXTENSION);
    std::optional<Assignment> a = assignments.findById(assignmentId);
    if (!a) throw NotFoundError("No assignment with that id.");
    std::optional<Course> course = courses.findCourse(a->getCourseId());
    if (!course || course->getLecturerId() != s.user().getId()) {
        throw AccessDeniedError("You are not the lecturer of this course.");
    }
    if (!courses.isEnrolled(studentId, a->getCourseId())) {
        throw ValidationError("That student is not enrolled on this course.");
    }
    if (newDeadline <= a->getDeadline()) {
        throw ValidationError("The new deadline must be later than the original deadline.");
    }
    requireNotBlank(reason, "Reason");

    assignments.saveExtension(
        Extension(0, assignmentId, studentId, newDeadline, reason, s.user().getId(), clock.now()));
}

std::vector<Assignment> AssignmentService::listForStudent(Session& s) {
    s.require(Action::VIEW_ASSIGNMENTS);
    return assignments.forStudent(s.user().getId());
}

std::vector<Assignment> AssignmentService::listForLecturer(Session& s, bool byDeadline) {
    s.require(Action::VIEW_OWN_ASSIGNMENTS);
    std::vector<Assignment> list = assignments.forLecturer(s.user().getId());  // by deadline
    if (!byDeadline) {
        std::sort(list.begin(), list.end(), [](const Assignment& x, const Assignment& y) {
            return x.getId() < y.getId();
        });
    }
    return list;
}

std::vector<Course> AssignmentService::coursesForStudent(Session& s) {
    s.require(Action::VIEW_COURSES);
    return courses.coursesForStudent(s.user().getId());
}

std::vector<Course> AssignmentService::coursesForLecturer(Session& s) {
    s.require(Action::VIEW_OWN_ASSIGNMENTS);
    return courses.coursesForLecturer(s.user().getId());
}

std::optional<Extension> AssignmentService::myExtension(Session& s, int assignmentId) {
    s.require(Action::VIEW_ASSIGNMENTS);
    return assignments.findExtension(assignmentId, s.user().getId());
}
