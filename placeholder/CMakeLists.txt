cmake_minimum_required(VERSION 3.9)
project(placeholder LANGUAGES CXX)

# When arbor PR is in, this will become find_package(arbor REQUIRED MPI)
find_package(arbor REQUIRED)

add_executable(placeholder placeholder.cpp)
target_link_libraries(placeholder PRIVATE arbor::arbor)

install(TARGETS placeholder DESTINATION bin)
