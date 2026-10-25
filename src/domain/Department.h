// Department.h - a school department, optionally headed by one HoD.
#pragma once

#include <optional>
#include <string>
#include <utility>

class Department {
public:
    Department(int id, std::string name, std::optional<int> hodUserId)
        : id(id), name(std::move(name)), hodUserId(hodUserId) {}

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::optional<int> getHodUserId() const { return hodUserId; }

private:
    int id;
    std::string name;
    std::optional<int> hodUserId;
};
