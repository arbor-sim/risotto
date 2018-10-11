# Expects variable install_dir to be defined.

if(NOT install_dir)
    message(FATAL_ERROR "Script requries install_dir variable be set.")
endif()

# Create .../lib/cmake/conduit

file(MAKE_DIRECTORY "${install_dir}/lib/cmake")

# Create a conduit-config.cmake in .../lib/cmake/conduit/ that
# * includes the generated export file ../lib/cmake/conduit.cmake,
# * defines CONDUIT_INCLUDE_DIRS for compatibility with Findconduit.cmake
# * adds the include directory property to the conduit target.

file(WRITE "${install_dir}/lib/cmake/conduit/conduit-config.cmake"
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/../conduit.cmake\")\n"
    "set(CONDUIT_INCLUDE_DIRS \"\${CMAKE_CURRENT_LIST_DIR}/../../../include\")\n"
    "set_target_properties(conduit conduit_relay conduit_blueprint PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"\${CONDUIT_INCLUDE_DIRS}\")\n")

