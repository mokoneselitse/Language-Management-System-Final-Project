#include "Menus.h"

std::unique_ptr<RoleMenu> MenuFactory::create(Session& session, ServiceRegistry& services,
                                              std::istream& in, std::ostream& out) {
    switch (session.user().getRole()) {
        case Role::STUDENT:
            return std::make_unique<StudentMenu>(session, services, in, out);
        case Role::LECTURER:
            return std::make_unique<LecturerMenu>(session, services, in, out);
        case Role::HOD:
            return std::make_unique<HodMenu>(session, services, in, out);
        case Role::ADMIN:
            return std::make_unique<AdminMenu>(session, services, in, out);
    }
    return nullptr;  // not reached
}
