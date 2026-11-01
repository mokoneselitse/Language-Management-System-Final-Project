cmake_minimum_required(VERSION 3.16)
project(ModeratedLMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------- domain layer
add_library(lms_domain STATIC
  src/domain/Types.cpp
  src/domain/User.cpp
  src/domain/Assignment.cpp
  src/domain/Submission.cpp
)
target_include_directories(lms_domain PUBLIC src/domain)

if(MSVC)
  target_compile_options(lms_domain PRIVATE /W4)
else()
  target_compile_options(lms_domain PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------- data layer
find_package(SQLite3 REQUIRED)
find_path(SODIUM_INCLUDE_DIR sodium.h REQUIRED)
find_library(SODIUM_LIBRARY NAMES sodium libsodium REQUIRED)

add_library(lms_data STATIC
  src/data/Statement.cpp
  src/data/SqliteDatabase.cpp
  src/data/SqliteUserRepository.cpp
  src/data/SqliteCourseRepository.cpp
  src/data/SqliteAssignmentRepository.cpp
  src/data/SqliteSubmissionRepository.cpp
  src/data/SqliteAuditRepository.cpp
  src/data/UserFactory.cpp
  src/data/SodiumPasswordHasher.cpp
)
target_include_directories(lms_data PUBLIC src/data PRIVATE ${SODIUM_INCLUDE_DIR})
# Newer CMake calls the SQLite target SQLite3::SQLite3, older CMake calls it SQLite::SQLite3.
if(TARGET SQLite3::SQLite3)
  set(SQLITE_TARGET SQLite3::SQLite3)
else()
  set(SQLITE_TARGET SQLite::SQLite3)
endif()
target_link_libraries(lms_data PUBLIC lms_domain ${SQLITE_TARGET} ${SODIUM_LIBRARY})

if(MSVC)
  target_compile_options(lms_data PRIVATE /W4)
else()
  target_compile_options(lms_data PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------- business logic layer
add_library(lms_logic STATIC
  src/logic/LatePenalty.cpp
  src/logic/Session.cpp
  src/logic/AuthService.cpp
  src/logic/AdminService.cpp
  src/logic/AssignmentService.cpp
  src/logic/SubmissionService.cpp
  src/logic/MarkingService.cpp
  src/logic/ModerationService.cpp
  src/logic/AuditService.cpp
)
target_include_directories(lms_logic PUBLIC src/logic)
target_link_libraries(lms_logic PUBLIC lms_data)

if(MSVC)
  target_compile_options(lms_logic PRIVATE /W4)
else()
  target_compile_options(lms_logic PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------- presentation layer + program
add_executable(lms
  src/main.cpp
  src/presentation/RoleMenu.cpp
  src/presentation/StudentMenu.cpp
  src/presentation/LecturerMenu.cpp
  src/presentation/HodMenu.cpp
  src/presentation/AdminMenu.cpp
  src/presentation/MenuFactory.cpp
  src/presentation/HiddenInput.cpp
)
target_include_directories(lms PRIVATE src/presentation)
target_link_libraries(lms PRIVATE lms_logic)

if(MSVC)
  target_compile_options(lms PRIVATE /W4)
else()
  target_compile_options(lms PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------- automated tests (Catch2)
option(LMS_BUILD_TESTS "Build the automated tests" ON)
if(LMS_BUILD_TESTS)
  find_package(Catch2 3 QUIET)
  if(Catch2_FOUND)
    enable_testing()
    add_executable(lms_tests
      tests/unit/test_late_penalty.cpp
      tests/unit/test_days_late.cpp
      tests/unit/test_submission_state.cpp
      tests/unit/test_users.cpp
      tests/unit/test_assignment.cpp
    )
    target_include_directories(lms_tests PRIVATE tests)
    target_link_libraries(lms_tests PRIVATE lms_logic Catch2::Catch2WithMain)
    include(Catch)
    catch_discover_tests(lms_tests)
  else()
    message(STATUS "Catch2 not found - the tests will not be built.")
  endif()
endif()
