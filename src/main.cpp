// main.cpp - the composition root.
// This is the only place that knows the concrete classes (SqliteDatabase,
// SodiumPasswordHasher, SystemClock, StandardLatePenalty). Everything else
// receives interfaces through its constructor (dependency injection).
#include <filesystem>
#include <iostream>
#include <string>

#include "Errors.h"
#include "HiddenInput.h"
#include "Menus.h"
#include "SodiumPasswordHasher.h"
#include "SqliteRepositories.h"

static std::string ask(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

// On the very first start the database has no users, so nobody could log in.
// The person installing the system creates the administrator account here.
static std::string askPassword(const std::string& prompt) {
    std::cout << prompt;
    return readHiddenLine(std::cin, std::cout);
}

static void createFirstAdmin(IUserRepository& users, IPasswordHasher& hasher) {
    if (users.findByUsername("admin") != nullptr) return;
    std::cout << "First start: please create the administrator account (username: admin).\n";
    std::string name = ask("Full name: ");
    std::string password = askPassword("Password: ");
    while (name.empty() || password.empty()) {
        if (!std::cin) throw LmsError("Input closed before the administrator was created.");
        std::cout << "Name and password cannot be empty.\n";
        name = ask("Full name: ");
        password = askPassword("Password: ");
    }
    users.add(Role::ADMIN, "admin", name, hasher.hash(password));
    std::cout << "Administrator created. You can now log in as 'admin'.\n";
}

int main(int argc, char* argv[]) {
    // Usage: lms [database file]. Submitted files are copied into a
    // "submissions" folder next to the database file.
    std::string dbPath = (argc > 1) ? argv[1] : "lms.db";
    std::filesystem::path dbFolder = std::filesystem::path(dbPath).parent_path();
    std::string storageFolder = (dbFolder / "submissions").string();

    try {
        // ---- data access layer
        SqliteDatabase db(dbPath);
        SqliteUserRepository users(db);
        SqliteCourseRepository courses(db);
        SqliteAssignmentRepository assignments(db);
        SqliteSubmissionRepository submissions(db);
        SqliteAuditRepository audit(db);
        SodiumPasswordHasher hasher;

        // ---- business logic layer
        SystemClock clock;
        StandardLatePenalty penalty;
        AuthService authService(users, hasher);
        AdminService adminService(users, courses, hasher, clock);
        AssignmentService assignmentService(assignments, courses, clock);
        SubmissionService submissionService(assignments, courses, submissions, clock,
                                            storageFolder);
        MarkingService markingService(submissions, assignments, audit, penalty, db, clock);
        ModerationService moderationService(submissions, assignments, courses, audit, db, clock);
        AuditService auditService(audit);
        ServiceRegistry services{authService,    adminService,      assignmentService,
                                 submissionService, markingService, moderationService,
                                 auditService};

        createFirstAdmin(users, hasher);

        // ---- presentation layer: log in, run the menu for that role, repeat
        std::cout << "\n===== Moderated LMS =====\n";
        while (std::cin) {
            std::string username = ask("\nUsername (leave empty to quit): ");
            if (username.empty()) break;
            std::string password = askPassword("Password: ");
            try {
                Session session = services.auth.login(username, password);
                std::cout << "Welcome, " << session.user().getFullName() << ".\n";
                std::unique_ptr<RoleMenu> menu =
                    MenuFactory::create(session, services, std::cin, std::cout);
                menu->run();
                std::cout << "Logged out.\n";
            } catch (const AuthenticationError& e) {
                std::cout << e.what() << "\n";
            } catch (const LmsError& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    } catch (const std::exception& e) {
        // e.g. the database file cannot be opened
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    std::cout << "Goodbye.\n";
    return 0;
}
