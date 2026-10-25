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
target_link_libraries(lms_data PUBLIC lms_domain SQLite::SQLite3 ${SODIUM_LIBRARY})

if(MSVC)
  target_compile_options(lms_data PRIVATE /W4)
else()
  target_compile_options(lms_data PRIVATE -Wall -Wextra -Wpedantic)
endif()
