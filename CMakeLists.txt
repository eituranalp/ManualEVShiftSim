cmake_minimum_required(VERSION 3.10)

# Project version and description
project(ManualEVShiftSim
    VERSION 0.1.0
    DESCRIPTION "Manual transmission EV simulation"
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
)

# Link libraries
target_link_libraries(ManualEVShiftSim 
    PRIVATE 
        ev_sim_core
        m
)

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
    RUNTIME DESTINATION bin
)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Project version: ${PROJECT_VERSION}")