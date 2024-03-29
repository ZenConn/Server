cmake_minimum_required(VERSION 3.14...3.22)

project(ServerTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

find_package(Boost 1.84.0 COMPONENTS filesystem regex json)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE BEAST_FILES ${Boost_INCLUDE_DIRS}/boost/beast/*.hpp
     ${Boost_INCLUDE_DIRS}/boost/beast/*.ipp
)
source_group(
  TREE ${Boost_INCLUDE_DIRS}/boost/beast
  PREFIX beast
  FILES ${BEAST_FILES}
)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

CPMAddPackage("gh:doctest/doctest@2.4.9")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

if(TEST_INSTALLED_VERSION)
  find_package(Server REQUIRED)
else()
  CPMAddPackage(NAME Server SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources} ${BEAST_FILES})
target_link_libraries(${PROJECT_NAME} doctest::doctest Server::Server OpenSSL::Crypto OpenSSL::SSL)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Server PUBLIC -Wall -Wpedantic -Wextra)
  elseif(MSVC)
    target_compile_options(Server PUBLIC /W4)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add WorkerTests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(Server PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Server PUBLIC -fprofile-arcs -ftest-coverage)
endif()
