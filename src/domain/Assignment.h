// Assignment.h - an assignment set for a course, with a deadline and a maximum mark.
#pragma once

#include <string>

#include "Extension.h"
#include "Types.h"

class Assignment {
public:
    Assignment(int id, int courseId, std::string title, std::string description,
               Timestamp deadline, double maxMark, int createdBy);

    int getId() const;
    int getCourseId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    Timestamp getDeadline() const;
    double getMaxMark() const;
    int getCreatedBy() const;

    // The deadline that applies to one student: their extension if they have
    // one (and it is later), otherwise the normal deadline. Pass nullptr if
    // the student has no extension.
    Timestamp effectiveDeadlineFor(const Extension* ext) const;

private:
    int id;
    int courseId;
    std::string title;
    std::string description;
    Timestamp deadline;
    double maxMark;
    int createdBy;
};
