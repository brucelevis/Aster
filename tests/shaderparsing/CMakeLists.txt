cmake_minimum_required(VERSION 3.19)

file(GLOB_RECURSE TESTS_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp" "${CMAKE_SOURCE_DIR}/tests/*.cc" "${CMAKE_SOURCE_DIR}/tests/*.h")

add_executable(TESTS_EXE ${TESTS_SOURCES})
target_link_libraries(TESTS_EXE Catch2::Catch2 RENDERING_LIB)

set_target_properties(TESTS_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")
