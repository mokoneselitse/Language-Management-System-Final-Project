#include "Assignment.h"

#include <utility>

Assignment::Assignment(int id, int courseId, std::string title, std::string description,
                       Timestamp deadline, double maxMark, int createdBy)
    : id(id),
      courseId(courseId),
      title(std::move(title)),
      description(std::move(description)),
      deadline(deadline),
      maxMark(maxMark),
      createdBy(createdBy) {}

int Assignment::getId() const { return id; }
int Assignment::getCourseId() const { return courseId; }
std::string Assignment::getTitle() const { return title; }
std::string Assignment::getDescription() const { return description; }
Timestamp Assignment::getDeadline() const { return deadline; }
double Assignment::getMaxMark() const { return maxMark; }
int Assignment::getCreatedBy() const { return createdBy; }

Timestamp Assignment::effectiveDeadlineFor(const Extension* ext) const {
    if (ext != nullptr && ext->getNewDeadline() > deadline) {
        return ext->getNewDeadline();
    }
    return deadline;
}
