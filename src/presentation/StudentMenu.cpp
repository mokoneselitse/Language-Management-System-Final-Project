#include "Menus.h"

void StudentMenu::showOptions() {
    out << "=== Student menu (" << session.user().getFullName() << ") ===\n"
        << "1. View my courses\n"
        << "2. View my assignments\n"
        << "3. Submit an assignment\n"
        << "4. View my results and feedback\n"
        << "0. Log out\n";
}

bool StudentMenu::handle(int choice) {
    switch (choice) {
        case 1: {  // FR14
            std::vector<Course> courses = services.assignments.coursesForStudent(session);
            if (courses.empty()) out << "You are not enrolled on any course yet.\n";
            for (const Course& c : courses) {
                out << "  " << c.getCode() << "  " << c.getTitle() << "\n";
            }
            return true;
        }
        case 2: {  // FR15
            std::vector<Assignment> list = services.assignments.listForStudent(session);
            if (list.empty()) out << "No assignments.\n";
            for (const Assignment& a : list) {
                out << "  [" << a.getId() << "] " << a.getTitle()
                    << "  | deadline " << formatTime(a.getDeadline());
                std::optional<Extension> ext = services.assignments.myExtension(session, a.getId());
                if (ext) {
                    out << " (extended to " << formatTime(ext->getNewDeadline()) << " for you)";
                }
                out << "  | max mark " << a.getMaxMark() << "\n";
            }
            return true;
        }
        case 3: {  // FR16, FR17
            int id = readInt("Assignment id: ");
            std::string path = readLine("Path to your file: ");
            Submission s = services.submissions.submit(session, id, path);
            out << "Submitted. Status: " << toString(s.getStatus())
                << ", attempt " << s.getAttemptNo();
            if (s.getDaysLate() > 0) out << ", " << s.getDaysLate() << " day(s) late";
            out << "\n";
            return true;
        }
        case 4: {  // FR09, FR13, FR19
            std::vector<StudentResult> results = services.submissions.resultsFor(session);
            if (results.empty()) out << "You have not submitted anything yet.\n";
            for (const StudentResult& r : results) {
                out << "  " << r.assignmentTitle << "  | " << toString(r.status);
                if (r.daysLate > 0) out << " (" << r.daysLate << " day(s) late)";
                out << "\n    Mark: ";
                if (r.mark) out << *r.mark << " / " << r.maxMark;
                else out << "not released yet";
                out << "\n    Feedback: " << (r.feedback ? *r.feedback : "none yet") << "\n";
            }
            return true;
        }
        case 0:
            return false;
        default:
            out << "Unknown option.\n";
            return true;
    }
}
