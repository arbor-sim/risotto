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
        -DARB_WITH_MPI=ON
        -DARB_VECTORIZE=${ARB_VECTORIZE}
        -DARB_ARCH=${ARB_ARCH})

# ZeroMQ C++ bindings
# -------------------

find_package(cppzmq QUIET)
message(STATUS "DEBUG cppzmq_FOUND: ${cppzmq_FOUND}")
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
# * The relevant CMakeLists.txt actually in conduit/src rather than
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

# Catch2 include directory work-around
# ------------------------------------
#
# Contra looks for catch2 headers under 'catch/' instead of
# 'catch2/'. This is a fragile work-around that copies the
# headers to ${install_dir}/include/catch, which should be
# found by contra if any dependency was installed via subproject.

if(Catch2_FOUND)
    get_target_property(catch2_include_dir Catch2::Catch2 INTERFACE_INCLUDE_DIRECTORIES)
else()
    set(catch2_include_dir "${install_dir}/include")
endif()
add_custom_target(catch2-work-around
    DEPENDS catch2
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${catch2_include_dir}/catch2" "${install_dir}/include/catch")


# Contra
# ------
#
# Build with support for zeromq, but omit shmem support.

find_package(contra QUIET)
add_cmake_subproject(
    contra
    TARGET contra
    UNLESS contra_FOUND
    GIT_SUBMODULE
    COMPILERS CXX
    INSTALL_DIR "${install_dir}"
    DEPENDS cppzmq conduit catch2 catch2-work-around
    CMAKE_ARGS
        -DWITH_TRANSPORT_ZEROMQ=ON
)

