#include "SqliteRepositories.h"

#include "SqlHelpers.h"
#include "UserFactory.h"

// The department id of a HoD is stored in department.hod_user_id, so it is
// read with a LEFT JOIN (it stays NULL for every other role).
static const char* SELECT_USER =
    "SELECT u.user_id, u.username, u.password_hash, u.full_name, u.role, u.is_active, "
    "       d.department_id "
    "FROM users u LEFT JOIN department d ON d.hod_user_id = u.user_id ";

static std::unique_ptr<User> readUser(Statement& st) {
    Role role = roleFromString(st.columnText(4));
    std::unique_ptr<User> user = UserFactory::create(role, st.columnInt(0), st.columnText(1),
                                                     st.columnText(3), st.columnText(2),
                                                     st.columnInt(5) == 1);
    if (role == Role::HOD) {
        auto* hod = static_cast<HeadOfDepartment*>(user.get());
        hod->setDepartmentId(readOptionalInt(st, 6));
    }
    return user;
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement st = db.prepare(std::string(SELECT_USER) + "WHERE u.username = ?;");
    st.bind(1, username);
    if (!st.step()) return nullptr;
    return readUser(st);
}

std::unique_ptr<User> SqliteUserRepository::findById(int id) {
    Statement st = db.prepare(std::string(SELECT_USER) + "WHERE u.user_id = ?;");
    st.bind(1, id);
    if (!st.step()) return nullptr;
    return readUser(st);
}

int SqliteUserRepository::add(Role role, const std::string& username, const std::string& fullName,
                              const std::string& hash) {
    Statement st = db.prepare(
        "INSERT INTO users (username, password_hash, full_name, role, is_active, created_at) "
        "VALUES (?, ?, ?, ?, 1, ?);");
    st.bind(1, username).bind(2, hash).bind(3, fullName).bind(4, toString(role)).bind(5, nowUnix());
    st.step();
    return db.lastInsertId();
}

void SqliteUserRepository::setActive(int id, bool active) {
    Statement st = db.prepare("UPDATE users SET is_active = ? WHERE user_id = ?;");
    st.bind(1, active ? 1 : 0).bind(2, id);
    st.step();
}
