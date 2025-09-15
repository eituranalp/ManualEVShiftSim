cmake_minimum_required(VERSION 3.10)

# Project version and description
project(ManualEVShiftSim
    VERSION 0.1.0
    DESCRIPTION "Manual transmission EV simulation with SDL3 controller input and ImGui dashboard"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Compiler warnings and settings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Find packages
find_package(SDL3 REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

# Set ImGui backend directory
set(IMGUI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui_backends")


# Add source files
add_library(ev_sim_core
    src/engine.cpp
    src/clutch.cpp
    # Future files from Task 002
    # src/driveline.cpp
    # src/input_loader.cpp
)

# Set include directories
target_include_directories(ev_sim_core 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Create main executable
add_executable(ManualEVShiftSim 
    main.cpp
    # Core ImGui source files
    ${IMGUI_BACKEND_DIR}/imgui.cpp
    ${IMGUI_BACKEND_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKEND_DIR}/imgui_draw.cpp
    ${IMGUI_BACKEND_DIR}/imgui_tables.cpp
    # ImGui backend files
    ${IMGUI_BACKEND_DIR}/imgui_impl_sdl3.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

# Add ImGui include directories to the main executable
target_include_directories(ManualEVShiftSim 
    PRIVATE
        "C:/Users/Eren T/vcpkg/installed/x64-windows/include"
)

# Link libraries - Now using compiled ImGui source files
target_link_libraries(ManualEVShiftSim 
    PRIVATE 
        ev_sim_core
        SDL3::SDL3
        OpenGL::GL
)

# Link libm only on non-MSVC toolchains
if(NOT MSVC)
    target_link_libraries(ManualEVShiftSim PRIVATE m)
endif()

# Enable testing
enable_testing()

# Add test executable (commented until tests are implemented)
# add_executable(ev_sim_tests
#     tests/engine_tests.cpp
#     tests/driveline_tests.cpp
#     tests/input_loader_tests.cpp
# )
# target_link_libraries(ev_sim_tests PRIVATE ev_sim_core)
# add_test(NAME unit_tests COMMAND ev_sim_tests)

# Install rules
install(TARGETS ManualEVShiftSim
    RUNTIME DESTINATION .
)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "SDL3 support: Enabled")
message(STATUS "ImGui support: Enabled")
message(STATUS "OpenGL support: Enabled")

## Documentation (Doxygen) removed at user request

# --- Runtime files and packaging ---
# Install license and default ImGui config at ZIP root
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    DESTINATION .
)

# Try to install SDL3 runtime DLL to ZIP root when available (Windows)
if(TARGET SDL3::SDL3)
    install(FILES $<TARGET_FILE:SDL3::SDL3> DESTINATION .)
endif()

## Note: Do not use install(CODE) to copy runtime files to absolute prefixes during CPack staging
## to avoid permission issues on Windows (e.g., Program Files).

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "ManualEVShiftSim")
set(CPACK_PACKAGE_VENDOR "Eren T")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
# Put files directly in the ZIP root (no top-level dir)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)