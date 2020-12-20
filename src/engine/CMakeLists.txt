cmake_minimum_required(VERSION 3.19)

file(GLOB_RECURSE SPIRV_SOURCES "${SPIRV_DEP_DIR}/*.cpp" "${SPIRV_DEP_DIR}/*.cc")
file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/src/engine/*.cpp" "${CMAKE_SOURCE_DIR}/src/engine/*.h" "${CMAKE_SOURCE_DIR}/src/engine/*.hpp")

add_library(SPIRV_LIB ${SPIRV_SOURCES})
set_target_properties(SPIRV_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)

add_library(ENGINE_LIB ${ENGINE_SOURCES})
set_target_properties(ENGINE_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)

target_link_libraries(ENGINE_LIB SPIRV_LIB)