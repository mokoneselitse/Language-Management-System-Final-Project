# Coding Standard

This is the coding standard followed by the Moderated LMS source code. It is
based on common C++ Core Guidelines practice, kept short for a project of this size.

## 1. Language and build
* C++17, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
* The code must build with **no warnings** using `-Wall -Wextra -Wpedantic`.
* Build with CMake only; each layer is a separate library.

## 2. Files
* One main class per file: `ClassName.h` and `ClassName.cpp`. Small, related classes may share a header
  (e.g. the four role menus in `Menus.h`, the repository interfaces in `Repositories.h`).
* Headers use `#pragma once`.
* Every header starts with a comment saying what the class is for.
* Include order: own header first, then standard library, then project headers.

## 3. Naming
| Thing | Style | Example |
|-------|-------|---------|
| Classes, structs, enums | PascalCase | `MarkingService`, `SubmissionStatus` |
| Interfaces | `I` + PascalCase | `IUserRepository`, `IClock` |
| Functions and methods | camelCase, starting with a verb | `recordMark()`, `findById()` |
| Variables and members | camelCase | `maxMark`, `daysLate` |
| Enum values | UPPER_CASE | `SubmissionStatus::RELEASED` |
| Constants | UPPER_CASE | `SCHEMA_SQL` |

Names should match the names in the design document (class diagrams and data dictionary).

## 4. Layout
* 4 spaces for indentation, no tabs. Lines at most 100 characters.
* Opening brace on the same line: `if (x) {`.
* One statement per line.

## 5. Classes
* Data members are `private` (or `protected` in a base class meant for inheritance).
* Getters are `const`.
* Base classes with virtual functions have a virtual destructor.
* Overriding functions are marked `override`.
* Dependencies are passed in through the constructor as references to interfaces
  (dependency injection). Only `main()` creates the concrete classes.

## 6. Errors and validation
* Errors are reported with exceptions from the `LmsError` hierarchy in `Errors.h`
  (`ValidationError`, `AccessDeniedError`, `NotFoundError`, ...). No error codes, no `exit()`.
* Every service method first calls `session.require(...)` to check permissions, then validates its input.
* The menus catch exceptions and show the message, so bad input never crashes the program.

## 7. Database and security
* All SQL lives in the data layer (`src/data`).
* SQL values are always bound to `?` placeholders through `Statement::bind()`.
  SQL is **never** built by adding user input to a string (prevents SQL injection).
* Changes that must happen together (a mark and its audit entry) run inside
  `IUnitOfWork::runInTransaction()`.
* Passwords are never stored or printed; only the Argon2id hash from `SodiumPasswordHasher` is saved.

## 8. Memory
* No raw `new`/`delete`. Use `std::unique_ptr` (with `std::make_unique`) for owned objects and
  `std::optional` for values that may be missing.
* Classes that own a resource (`SqliteDatabase`, `Statement`) free it in their destructor (RAII)
  and cannot be copied.

## 9. Comments
* Explain *why*, not *what* the code obviously does.
* Reference the requirement where a rule comes from, e.g. `// FR13` or `// C1`.

## 10. Git
* Commit messages are short and say what changed, e.g. `Phase 4: data access layer`.
* Build files (`build/`), databases (`*.db`) and submitted files (`submissions/`) are not committed.
