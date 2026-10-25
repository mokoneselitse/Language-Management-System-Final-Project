// ServiceRegistry.h - one place that holds all the services, so a menu only
// needs this struct instead of seven separate constructor parameters.
#pragma once

#include "AdminService.h"
#include "AssignmentService.h"
#include "AuditService.h"
#include "AuthService.h"
#include "MarkingService.h"
#include "ModerationService.h"
#include "SubmissionService.h"

struct ServiceRegistry {
    AuthService& auth;
    AdminService& admin;
    AssignmentService& assignments;
    SubmissionService& submissions;
    MarkingService& marking;
    ModerationService& moderation;
    AuditService& audit;
};
