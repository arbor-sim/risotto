# External dependencies for projects under risotto.

set(install_dir ${CMAKE_CURRENT_BINARY_DIR})

# Arbor
# -----
#
# Forward some configuration options on to Arbor config.

option(ARB_VECTORIZE "Enable explicit vectorization in Arbor" OFF)
set(ARB_ARCH "" CACHE STRING "Specify target architecture for Arbor")

find_package(arbor QUIET)
add_cmake_subproject(
    arbor
    UNLESS arbor_FOUND
    GIT_SUBMODULE
    INSTALL_DIR "${install_dir}"
    COMPILERS CXX
    CMAKE_ARGS
        -DARB_WITH_MPI=OFF
        -DARB_VECTORIZE=${ARB_VECTORIZE}
        -DARB_ARCH=${ARB_ARCH})

# ZeroMQ C++ bindings
# -------------------

find_package(cppzmq QUIET)
add_cmake_subproject(
    cppzmq
    UNLESS cppzmq_FOUND
    GIT_SUBMODULE
    COMPILERS CXX C
    INSTALL_DIR "${install_dir}"
    CMAKE_ARGS
        -DENABLE_DRAFTS=OFF
        -DCPPZMQ_BUILD_TESTS=OFF)

# Conduit
# -------
#
# Note:
#
# * The relevant CMakeLists.txt is actually in conduit/src rather than
#   in the top level directory.
#
# * The generated export cmake files are installed in a location where
#   CMake cannot find them, and thus need to be moved.
#
# * No conduit-config.cmake file is generated for use with find_package().
#
# Use supplied cmake script to patch up the install.

find_package(conduit QUIET)
add_cmake_subproject(
    conduit
    UNLESS conduit_FOUND
    SOURCE_DIR "conduit/src"
    GIT_SUBMODULE
    COMPILERS CXX C
    INSTALL_DIR "${install_dir}"
    CMAKE_ARGS
        -DENABLE_TESTS=OFF
        -DENABLE_DOCS=OFF
        -DCMAKE_SKIP_RPATH=ON
    POST_INSTALL
        ${CMAKE_COMMAND} "-Dinstall_dir=${install_dir}" -P "${CMAKE_CURRENT_SOURCE_DIR}/fix-conduit-install.cmake")

# Catch2
# ------
#
# Prerequisite for contra library.

find_package(Catch2 QUIET)
add_cmake_subproject(
    catch2
    UNLESS Catch2_FOUND
    GIT_SUBMODULE
    INSTALL_DIR "${install_dir}"
    CMAKE_ARGS
        -DCATCH_BUILD_TESTING=OFF
        -DCATCH_BUILD_EXAMPLES=OFF
        -DCATCH_INSTALL_DOCS=OFF
        -DCATCH_INSTALL_HELPERS=OFF
)


# Contra
# ------
#
# Currently using a fork (arbor-sim/contra) to work-around various CMake/conan-related issues.
# Build with support for zeromq, but omit shmem support.

find_package(contra QUIET)
add_cmake_subproject(
    contra
    UNLESS contra_FOUND
    GIT_SUBMODULE
    COMPILERS CXX
    INSTALL_DIR "${install_dir}"
    DEPENDS cppzmq conduit catch2
    CMAKE_ARGS
        -DWITH_ZEROMQ=ON)

# Nesci
# -----
#
# Currently using a fork (arbor-sim/nesci) to work-around various CMake/conan-related issues.

find_package(nesci QUIET)
add_cmake_subproject(
    nesci
    UNLESS nesci_FOUND
    GIT_SUBMODULE
    COMPILERS CXX
    INSTALL_DIR "${install_dir}"
    DEPENDS conduit catch2
)

