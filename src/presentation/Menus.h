// Menus.h - one menu per role. Each shows only that role's options (FR21);
// the services still check permissions again through Session::require().
#pragma once

#include <memory>

#include "RoleMenu.h"

class StudentMenu : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    void showOptions() override;
    bool handle(int choice) override;
};

class LecturerMenu : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    void showOptions() override;
    bool handle(int choice) override;
};

class HodMenu : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    void showOptions() override;
    bool handle(int choice) override;
};

class AdminMenu : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    void showOptions() override;
    bool handle(int choice) override;
};

// Factory: picks the menu subclass from the logged-in user's role,
// so main() never has to switch on the role.
class MenuFactory {
public:
    static std::unique_ptr<RoleMenu> create(Session& session, ServiceRegistry& services,
                                            std::istream& in, std::ostream& out);
};
