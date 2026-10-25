#include "Menus.h"

void LecturerMenu::showOptions() {
    out << "=== Lecturer menu (" << session.user().getFullName() << ") ===\n"
        << "1. View my courses\n"
        << "2. View my assignments\n"
        << "3. Create an assignment\n"
        << "4. Grant a deadline extension\n"
        << "5. View submissions for an assignment\n"
        << "6. Mark a submission\n"
        << "7. Re-mark a submission\n"
        << "0. Log out\n";
}

bool LecturerMenu::handle(int choice) {
    switch (choice) {
        case 1: {
            std::vector<Course> list = services.assignments.coursesForLecturer(session);
            if (list.empty()) out << "You are not assigned to any course yet.\n";
            for (const Course& c : list) {
                out << "  [" << c.getId() << "] " << c.getCode() << "  " << c.getTitle() << "\n";
            }
            return true;
        }
        case 2: {  // FR23
            int order = readInt("Sort by 1 = deadline, 2 = date created: ");
            std::vector<Assignment> list =
                services.assignments.listForLecturer(session, order != 2);
            if (list.empty()) out << "You have not created any assignments.\n";
            for (const Assignment& a : list) {
                out << "  [" << a.getId() << "] " << a.getTitle() << "  | course id "
                    << a.getCourseId() << "  | deadline " << formatTime(a.getDeadline())
                    << "  | max " << a.getMaxMark() << "\n";
            }
            return true;
        }
        case 3: {  // FR05
            int courseId = readInt("Course id (see option 1): ");
            std::string title = readLine("Title: ");
            std::string desc = readLine("Description: ");
            Timestamp deadline = readDateTime("Deadline (YYYY-MM-DD HH:MM): ");
            double maxMark = readDouble("Maximum mark: ");
            int id = services.assignments.createAssignment(session, courseId, title, desc,
                                                           deadline, maxMark);
            out << "Assignment created with id " << id << ".\n";
            return true;
        }
        case 4: {  // FR06
            int assignmentId = readInt("Assignment id: ");
            int studentId = readInt("Student id: ");
            Timestamp newDeadline = readDateTime("New deadline (YYYY-MM-DD HH:MM): ");
            std::string reason = readLine("Reason: ");
            services.assignments.grantExtension(session, assignmentId, studentId, newDeadline,
                                                reason);
            out << "Extension granted.\n";
            return true;
        }
        case 5: {
            int assignmentId = readInt("Assignment id: ");
            std::vector<Submission> list = services.marking.submissionsFor(session, assignmentId);
            if (list.empty()) out << "No submissions waiting.\n";
            for (const Submission& s : list) {
                out << "  [" << s.getId() << "] " << services.auth.nameOf(session, s.getStudentId())
                    << " (id " << s.getStudentId() << ")  | "
                    << toString(s.getStatus()) << "  | days late " << s.getDaysLate()
                    << "  | attempt " << s.getAttemptNo() << "  | file " << s.getFilePath();
                if (s.getMark()) out << "  | mark " << *s.getMark();
                out << "\n";
            }
            return true;
        }
        case 6: {  // FR07, FR08, FR18
            int id = readInt("Submission id: ");
            double raw = readDouble("Mark (before any late penalty): ");
            std::string feedback = readLine("Feedback: ");
            services.marking.mark(session, id, raw, feedback);
            out << "Marked. The student can now read the feedback; the mark stays hidden "
                   "until the HoD releases it.\n";
            return true;
        }
        case 7: {  // FR22
            int id = readInt("Submission id: ");
            double raw = readDouble("New mark (before any late penalty): ");
            std::string reason = readLine("Reason for the change: ");
            services.marking.remark(session, id, raw, reason);
            out << "Re-marked. The change is recorded in the audit log.\n";
            return true;
        }
        case 0:
            return false;
        default:
            out << "Unknown option.\n";
            return true;
    }
}
