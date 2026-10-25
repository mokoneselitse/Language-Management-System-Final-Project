#include "RoleMenu.h"

#include <ctime>
#include <iomanip>
#include <sstream>

#include "Errors.h"
#include "HiddenInput.h"

RoleMenu::RoleMenu(Session& session, ServiceRegistry& services, std::istream& in,
                   std::ostream& out)
    : session(session), services(services), in(in), out(out) {}

void RoleMenu::run() {
    bool keepGoing = true;
    while (keepGoing) {
        out << "\n";
        showOptions();
        int choice = readInt("Choice: ");
        try {
            keepGoing = handle(choice);
        } catch (const LmsError& e) {
            // Every rule the services break comes back as an LmsError,
            // so the user sees the message and the menu carries on.
            out << "Error: " << e.what() << "\n";
        } catch (const std::exception& e) {
            // Anything unexpected (e.g. from the standard library) must not crash the program.
            out << "Unexpected error: " << e.what() << "\n";
        }
        if (!in) keepGoing = false;  // input closed (e.g. Ctrl+Z / Ctrl+D)
    }
}

std::string RoleMenu::readLine(const std::string& prompt) {
    out << prompt;
    std::string line;
    if (!std::getline(in, line)) return "";
    if (!line.empty() && line.back() == '\r') line.pop_back();  // Windows line endings
    return line;
}

std::string RoleMenu::readPassword(const std::string& prompt) {
    out << prompt;
    return readHiddenLine(in, out);
}

int RoleMenu::readInt(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt);
        if (!in) return 0;
        try {
            size_t used = 0;
            int value = std::stoi(line, &used);
            if (used == line.size()) return value;
        } catch (const std::exception&) {
        }
        out << "Please enter a whole number.\n";
    }
}

double RoleMenu::readDouble(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt);
        if (!in) return 0;
        try {
            size_t used = 0;
            double value = std::stod(line, &used);
            if (used == line.size()) return value;
        } catch (const std::exception&) {
        }
        out << "Please enter a number, e.g. 42 or 42.5.\n";
    }
}

Timestamp RoleMenu::readDateTime(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt);
        if (!in) return Timestamp{};
        std::tm tm{};
        std::istringstream text(line);
        text >> std::get_time(&tm, "%Y-%m-%d %H:%M");
        if (!text.fail()) {
            tm.tm_isdst = -1;  // let mktime work out daylight saving
            std::time_t t = std::mktime(&tm);
            if (t != -1) return std::chrono::system_clock::from_time_t(t);
        }
        out << "Please use the format YYYY-MM-DD HH:MM, e.g. 2026-11-07 23:59.\n";
    }
}

std::string RoleMenu::formatTime(Timestamp t) {
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream text;
    text << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return text.str();
}

void RoleMenu::printAuditLog(const std::vector<AuditEntry>& entries) {
    if (entries.empty()) out << "No changes recorded for this submission.\n";
    for (const AuditEntry& e : entries) {
        out << "  " << formatTime(e.getChangedAt()) << "  " << toString(e.getAction())
            << " by " << services.auth.nameOf(session, e.getChangedBy()) << ": ";
        if (e.getOldMark()) out << *e.getOldMark(); else out << "-";
        out << " -> " << e.getNewMark() << "  (" << e.getReason() << ")\n";
    }
}
