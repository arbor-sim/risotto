# Super-project for interop and i/o examples.

cmake_minimum_required(VERSION 3.9)
project(risotto LANGUAGES CXX)

# The external subdirectory collects external dependencies with fallbacks to
# source builds via git submodules.

set(external_root "${CMAKE_BINARY_DIR}/external")
list(INSERT CMAKE_PREFIX_PATH 0 "${external_root}")

# (Conditionally) build and install CMake-based subproject.
# Arguments:
#     sub                Name and subdirectory of subproject.
#     GIT_SUBMODULE      Confirm git submodule is available or else
#                        substitue a dummy error target.
#     SOURCE_DIR dir     Path to top-level directory for subproject
#                        (default: ${CMAKE_CURRENT_SOURCE_DIR}/${sub}).
#     INSTALL_DIR dir    Installation prefix for subproject.
#                        (default: ${CMAKE_CURRENT_BINARY_DIR}/${sub}).
#     COMPILERS lang...  Forwrd definitions of compilers CMAKE_${lang}_COMPILER.
#     TARGET name        Name of generated target.
#     UNLESS var         Generate a dummy target if ${var} is true.
#     CMAKE_ARGS arg...  Extra arguments for subproject CMake.
#     DEPENDS target...  Add dependencies for the build target.

include(ExternalProject)
function(add_cmake_subproject sub)
    cmake_parse_arguments(arg
        "GIT_SUBMODULE"
        "SOURCE_DIR;INSTALL_DIR;TARGET;UNLESS"
        "CMAKE_ARGS;DEPENDS;COMPILERS"
        ${ARGN})

    get_filename_component(_source_dir "${sub}" ABSOLUTE)
    if(arg_SOURCE_DIR)
        set(_source_dir "${arg_SOURCE_DIR}")
    endif()

    get_filename_component(_install_dir "${sub}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    if(arg_INSTALL_DIR)
        set(_install_dir "${arg_INSTALL_DIR}")
    endif()

    set(_target "${sub}")
    if(arg_TARGET)
        set(_target "${arg_TARGET}")
    endif()

    get_filename_component(_abs_sub_path "${sub}" ABSOLUTE)
    set(_available TRUE)

    if(arg_GIT_SUBMODULE AND (NOT EXISTS "${_abs_sub_path}/.git"))
        set(_available FALSE)
        set(_reason "Missing git repository at ${_path}; require git submodule update?")
    endif()

    if(${arg_UNLESS})
        add_custom_target("${_target}" COMMAND true)
    elseif(NOT _available)
        add_custom_target("${_target}"
            COMMAND echo "  Error: ${_reason}."
            COMMAND exit 1)
    else()
        set(_cmake_module_path ${CMAKE_MODULE_PATH})
        set(_cmake_prefix_path ${CMAKE_PREFIX_PATH})
        string(REPLACE ";" ":" _cmake_module_path "${_cmake_module_path}")
        string(REPLACE ";" ":" _cmake_prefix_path "${_cmake_prefix_path}")

        set(_cmake_args
            "-DCMAKE_MODULE_PATH=${_cmake_module_path}"
            "-DCMAKE_PREFIX_PATH=${_cmake_prefix_path}")

        foreach(_lang ${arg_COMPILERS})
            list(APPEND _cmake_args
                "-DCMAKE_${_lang}_COMPILER=${CMAKE_${_lang}_COMPILER}")
        endforeach()

        list(APPEND _cmake_args
            "-DCMAKE_INSTALL_PREFIX=${_install_dir}"
            "-DCMAKE_BUILD_TYPE=release"
            ${arg_CMAKE_ARGS})

        ExternalProject_Add(${_target}
            SOURCE_DIR ${_source_dir}
            INSTALL_DIR "${_install_dir}"
            STEP_TARGETS build
            DEPENDS ${arg_DEPENDS}
            LIST_SEPARATOR :
            CMAKE_ARGS "${_cmake_args}"
        )
    endif()
endfunction()

add_subdirectory(external)

# Placeholder executable while we prototype the external stuff above.

add_cmake_subproject(placeholder COMPILERS CXX DEPENDS arbor)

