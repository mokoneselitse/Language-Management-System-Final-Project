#include "Menus.h"

#include "Errors.h"

// Turns the number the admin typed into a Role.
static Role roleFromNumber(int number) {
    switch (number) {
        case 1: return Role::STUDENT;
        case 2: return Role::LECTURER;
        case 3: return Role::HOD;
        case 4: return Role::ADMIN;
        default: throw ValidationError("Role must be 1, 2, 3 or 4.");
    }
}

void AdminMenu::showOptions() {
    out << "=== Administrator menu (" << session.user().getFullName() << ") ===\n"
        << "-- Users --\n"
        << " 1. Create a user\n"
        << " 2. Edit a user\n"
        << " 3. Reset a user's password\n"
        << " 4. Activate / deactivate a user\n"
        << " 5. List all users\n"
        << "-- Departments and courses --\n"
        << " 6. Create a department\n"
        << " 7. Change a department's HoD\n"
        << " 8. List departments\n"
        << " 9. Create a course\n"
        << "10. Edit a course\n"
        << "11. List courses\n"
        << "12. Assign a lecturer to a course\n"
        << "13. Enrol a student on a course\n"
        << "-- Other --\n"
        << "14. View the audit log of a submission\n"
        << " 0. Log out\n";
}

bool AdminMenu::handle(int choice) {
    switch (choice) {
        case 1: {  // FR01
            Role role =
                roleFromNumber(readInt("Role (1 = Student, 2 = Lecturer, 3 = HoD, 4 = Admin): "));
            std::string username = readLine("Username: ");
            std::string name = readLine("Full name: ");
            std::string password = readPassword("Password: ");
            int id = services.admin.createUser(session, role, username, name, password);
            out << "User created with id " << id << ".\n";
            return true;
        }
        case 2: {  // FR01
            int userId = readInt("User id: ");
            std::string username = readLine("New username: ");
            std::string name = readLine("New full name: ");
            services.admin.editUser(session, userId, username, name);
            out << "User updated.\n";
            return true;
        }
        case 3: {  // FR01
            int userId = readInt("User id: ");
            std::string password = readPassword("New password: ");
            services.admin.resetPassword(session, userId, password);
            out << "Password changed.\n";
            return true;
        }
        case 4: {  // FR01
            int userId = readInt("User id: ");
            int active = readInt("1 = activate, 0 = deactivate: ");
            services.admin.setUserActive(session, userId, active == 1);
            out << (active == 1 ? "User activated.\n" : "User deactivated.\n");
            return true;
        }
        case 5: {
            for (const std::unique_ptr<User>& u : services.admin.listUsers(session)) {
                out << "  [" << u->getId() << "] " << u->getUsername() << "  | "
                    << u->getFullName() << "  | " << toString(u->getRole())
                    << (u->isActive() ? "" : "  | DEACTIVATED") << "\n";
            }
            return true;
        }
        case 6: {  // FR02
            std::string name = readLine("Department name: ");
            int hodId = readInt("HoD user id (0 = none yet): ");
            std::optional<int> hod;
            if (hodId != 0) hod = hodId;
            int id = services.admin.createDepartment(session, name, hod);
            out << "Department created with id " << id << ".\n";
            return true;
        }
        case 7: {  // FR02
            int departmentId = readInt("Department id: ");
            int hodId = readInt("New HoD user id (0 = no HoD): ");
            std::optional<int> hod;
            if (hodId != 0) hod = hodId;
            services.admin.setDepartmentHod(session, departmentId, hod);
            out << "Department updated.\n";
            return true;
        }
        case 8: {
            std::vector<Department> list = services.admin.listDepartments(session);
            if (list.empty()) out << "No departments yet.\n";
            for (const Department& d : list) {
                out << "  [" << d.getId() << "] " << d.getName() << "  | HoD: "
                    << (d.getHodUserId() ? services.auth.nameOf(session, *d.getHodUserId())
                                         : std::string("none"))
                    << "\n";
            }
            return true;
        }
        case 9: {  // FR02
            std::string code = readLine("Course code: ");
            std::string title = readLine("Course title: ");
            int departmentId = readInt("Department id: ");
            int id = services.admin.createCourse(session, code, title, departmentId);
            out << "Course created with id " << id << ".\n";
            return true;
        }
        case 10: {  // FR02
            int courseId = readInt("Course id: ");
            std::string code = readLine("New course code: ");
            std::string title = readLine("New course title: ");
            services.admin.editCourse(session, courseId, code, title);
            out << "Course updated.\n";
            return true;
        }
        case 11: {
            std::vector<Course> list = services.admin.listCourses(session);
            if (list.empty()) out << "No courses yet.\n";
            for (const Course& c : list) {
                out << "  [" << c.getId() << "] " << c.getCode() << "  " << c.getTitle()
                    << "  | department " << c.getDepartmentId() << "  | lecturer: "
                    << (c.getLecturerId() ? services.auth.nameOf(session, *c.getLecturerId())
                                          : std::string("not assigned"))
                    << "\n";
            }
            return true;
        }
        case 12: {  // FR03
            int courseId = readInt("Course id: ");
            int lecturerId = readInt("Lecturer user id: ");
            services.admin.assignLecturer(session, courseId, lecturerId);
            out << "Lecturer assigned.\n";
            return true;
        }
        case 13: {  // FR04
            int studentId = readInt("Student user id: ");
            int courseId = readInt("Course id: ");
            services.admin.enrolStudent(session, studentId, courseId);
            out << "Student enrolled.\n";
            return true;
        }
        case 14: {  // FR22, O1
            int id = readInt("Submission id: ");
            printAuditLog(services.audit.history(session, id));
            return true;
        }
        case 0:
            return false;
        default:
            out << "Unknown option.\n";
            return true;
    }
}
