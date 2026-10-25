// RoleMenu.h - base class of the four role menus (Template Method).
// run() is the same loop for every role: show the options, read a choice,
// handle it. Each subclass only fills in showOptions() and handle().
#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ServiceRegistry.h"
#include "Session.h"

class RoleMenu {
public:
    RoleMenu(Session& session, ServiceRegistry& services, std::istream& in, std::ostream& out);
    virtual ~RoleMenu() = default;

    void run();

protected:
    virtual void showOptions() = 0;
    virtual bool handle(int choice) = 0;  // returns false when the user logs out

    int readInt(const std::string& prompt);
    std::string readLine(const std::string& prompt);
    // Extra input helpers added during coding:
    double readDouble(const std::string& prompt);
    std::string readPassword(const std::string& prompt);  // not shown on screen
    Timestamp readDateTime(const std::string& prompt);  // "YYYY-MM-DD HH:MM", local time
    static std::string formatTime(Timestamp t);
    void printAuditLog(const std::vector<AuditEntry>& entries);  // used by HoD and Admin

    Session& session;
    ServiceRegistry& services;
    std::istream& in;
    std::ostream& out;
};
