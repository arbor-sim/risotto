# External dependencies for projects under risotto.

set(install_dir ${CMAKE_CURRENT_BINARY_DIR}) 

# Arbor

find_package(arbor QUIET)
add_cmake_subproject(
    arbor
    UNLESS arbor_FOUND
    GIT_SUBMODULE
    INSTALL_dir "${install_dir}"
    COMPILERS CXX
    CMAKE_ARGS -DARB_WITH_MPI=ON)
