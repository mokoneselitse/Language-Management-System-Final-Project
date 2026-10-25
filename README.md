# Moderated LMS

A console-based Learning Management System for a school, with four roles
(Administrator, Lecturer, Head of Department, Student). Lecturers mark work,
the Head of Department moderates and releases the marks, and every mark
change is recorded in an audit log.

CS4433 Software Engineering – Final Project. Written in C++17 with SQLite and libsodium, built with CMake.

---

## 1. What you need

| Tool | Used for |
|------|----------|
| C++17 compiler (GCC 9 or newer) | compiling the code |
| CMake 3.16 or newer + Ninja | building |
| SQLite 3 | the database (`lms.db`) |
| libsodium | password hashing (Argon2id) |
| Git | version control |

## 2. Build on Windows (MSYS2)

1. Install MSYS2 from <https://www.msys2.org> (default folder `C:\msys64`).
2. Open the **MSYS2 UCRT64** terminal from the Start menu. The prompt must show `UCRT64`.
3. Update MSYS2. Run this, let it close the window, then open UCRT64 again and run it a second time:
   ```
   pacman -Syu
   ```
4. Install the tools:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium git
   ```
   On a slow connection, add `--disable-download-timeout` after `-S`.
5. Get the code and build it:
   ```
   git clone <repository-url> lms
   cd lms
   cmake -S . -B build -G Ninja
   cmake --build build
   ```
   The last line should say `Linking CXX executable lms.exe`.

## 3. Build on Linux (Ubuntu/Debian)

```
sudo apt install g++ cmake ninja-build libsqlite3-dev libsodium-dev git
cmake -S . -B build -G Ninja
cmake --build build
```

## 4. Run

```
./build/lms.exe              (Windows, in the UCRT64 terminal)
./build/lms                  (Linux)
./build/lms.exe school.db    (use a different database file)
```

* The database file is created automatically on the first run (default `lms.db` in the current folder).
* Submitted files are copied into a `submissions` folder next to the database file.
* **First start:** the program asks you to create the administrator account (username `admin`).
  Log in as `admin`, then create the other users, departments and courses from the Administrator menu.
* To quit, leave the username empty and press Enter.

> Passwords are hidden while you type. If they are still visible in the MSYS2 window,
> run the program from Windows Terminal or Command Prompt instead (the MSYS2 window is not a
> normal Windows console).

## 5. Project structure

The code follows the four-layer architecture from the design document. Each layer only uses the layer below it.

```
src/
  domain/        entities and rules: User and its 4 roles, Assignment, Submission (state machine), errors
  data/          SQLite access: SqliteDatabase, Statement, repositories, schema, UserFactory, password hashing
  logic/         services (Auth, Admin, Assignment, Submission, Marking, Moderation, Audit), Session,
                 late-penalty strategy, clock
  presentation/  RoleMenu and the four role menus, MenuFactory
  main.cpp       creates all objects and runs the login loop
```

CMake builds each layer as its own library (`lms_domain`, `lms_data`, `lms_logic`), so a layer cannot use
a layer above it by mistake.

## 6. Coding standard

See [CODING_STANDARD.md](CODING_STANDARD.md).
