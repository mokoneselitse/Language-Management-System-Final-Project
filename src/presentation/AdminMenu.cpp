#include "Menus.h"

#include "Errors.h"

// Asks for a role by number and returns it.
static Role askRole(int number) {
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
        << "1. Create a user\n"
        << "2. Activate / deactivate a user\n"
        << "3. Create a department\n"
        << "4. Create a course\n"
        << "5. Assign a lecturer to a course\n"
        << "6. Enrol a student on a course\n"
        << "7. View the audit log of a submission\n"
        << "0. Log out\n";
}

// The new ids are printed after each create, because the other options
// (assign, enrol, ...) ask for them.
bool AdminMenu::handle(int choice) {
    switch (choice) {
        case 1: {  // FR01
            Role role = askRole(readInt("Role (1 = Student, 2 = Lecturer, 3 = HoD, 4 = Admin): "));
            std::string username = readLine("Username: ");
            std::string name = readLine("Full name: ");
            std::string password = readLine("Password: ");
            int id = services.admin.createUser(session, role, username, name, password);
            out << "User created with id " << id << ".\n";
            return true;
        }
        case 2: {  // FR01
            int userId = readInt("User id: ");
            int active = readInt("1 = activate, 0 = deactivate: ");
            services.admin.setUserActive(session, userId, active == 1);
            out << (active == 1 ? "User activated.\n" : "User deactivated.\n");
            return true;
        }
        case 3: {  // FR02
            std::string name = readLine("Department name: ");
            int hodId = readInt("HoD user id (0 = none yet): ");
            std::optional<int> hod;
            if (hodId != 0) hod = hodId;
            int id = services.admin.createDepartment(session, name, hod);
            out << "Department created with id " << id << ".\n";
            return true;
        }
        case 4: {  // FR02
            std::string code = readLine("Course code: ");
            std::string title = readLine("Course title: ");
            int departmentId = readInt("Department id: ");
            int id = services.admin.createCourse(session, code, title, departmentId);
            out << "Course created with id " << id << ".\n";
            return true;
        }
        case 5: {  // FR03
            int courseId = readInt("Course id: ");
            int lecturerId = readInt("Lecturer user id: ");
            services.admin.assignLecturer(session, courseId, lecturerId);
            out << "Lecturer assigned.\n";
            return true;
        }
        case 6: {  // FR04
            int studentId = readInt("Student user id: ");
            int courseId = readInt("Course id: ");
            services.admin.enrolStudent(session, studentId, courseId);
            out << "Student enrolled.\n";
            return true;
        }
        case 7: {  // FR22, O1
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
