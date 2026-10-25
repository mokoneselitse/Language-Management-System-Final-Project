// Course.h - a course offered by a department and taught by (at most) one lecturer.
#pragma once

#include <optional>
#include <string>
#include <utility>

class Course {
public:
    Course(int id, std::string code, std::string title, int departmentId,
           std::optional<int> lecturerId)
        : id(id),
          code(std::move(code)),
          title(std::move(title)),
          departmentId(departmentId),
          lecturerId(lecturerId) {}

    int getId() const { return id; }
    std::string getCode() const { return code; }
    std::string getTitle() const { return title; }
    int getDepartmentId() const { return departmentId; }
    std::optional<int> getLecturerId() const { return lecturerId; }

private:
    int id;
    std::string code;
    std::string title;
    int departmentId;
    std::optional<int> lecturerId;
};
