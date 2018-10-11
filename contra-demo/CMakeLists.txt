cmake_minimum_required(VERSION 3.9)
project(contra-demo LANGUAGES CXX)

find_package(arbor REQUIRED)
find_package(conduit REQUIRED) # TODO: conduit and cppzmq dependencies should be in the contra or nesci config.cmake.
find_package(cppzmq REQUIRED)
find_package(contra REQUIRED core zmq)
find_package(nesci REQUIRED layout producer)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(contra-demo contra_demo.cpp ring_recipe.cpp)
target_link_libraries(contra-demo PRIVATE arbor::arbor contra_zmq nesci_producer)

install(TARGETS contra-demo DESTINATION bin)
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/contra-demo-client.py" DESTINATION bin)
