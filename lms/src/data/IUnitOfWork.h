// IUnitOfWork.h - runs several database changes as one transaction.
// Used so a mark change and its audit entry are saved together or not at all.
#pragma once

#include <functional>

class IUnitOfWork {
public:
    virtual ~IUnitOfWork() = default;
    virtual void runInTransaction(std::function<void()> work) = 0;
};
