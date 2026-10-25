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
