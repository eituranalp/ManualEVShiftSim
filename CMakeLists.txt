cmake_minimum_required(VERSION 3.10)

project(ManualEVShiftSim)

add_executable(main main.cpp) #(generated file, source files)

target_link_libraries(main PRIVATE m) #imported libraries