cmake_minimum_required(VERSION 3.14...3.22)

project(ServerStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(NAME Server SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

find_package(Boost 1.84.0 COMPONENTS filesystem regex)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

file(GLOB_RECURSE BEAST_FILES ${Boost_INCLUDE_DIRS}/boost/beast/*.hpp ${Boost_INCLUDE_DIRS}/boost/beast/*.ipp)

source_group(TREE ${Boost_INCLUDE_DIRS}/boost/beast PREFIX beast FILES ${BEAST_FILES})

add_executable(${PROJECT_NAME} ${sources} ${BEAST_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "Server")

target_link_libraries(${PROJECT_NAME} Server::Server cxxopts)
