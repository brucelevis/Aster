cmake_minimum_required(VERSION 3.19)

project(Aster VERSION 0.1
        DESCRIPTION "<=*_*=>"
        LANGUAGES CXX)


find_package(Vulkan)

set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(DEP_DIR "${CMAKE_SOURCE_DIR}/dependency")
set(GLFW_DEP_DIR "${DEP_DIR}/glfw")
set(GLM_DEP_DIR "${DEP_DIR}/glm")
set(SPIRV_DEP_DIR "${DEP_DIR}/spirv_cross")
set(CATCH2_DEP_DIR "${DEP_DIR}/Catch2")

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib" "${Vulkan_LIBRARIES}")

file(GLOB_RECURSE SPIRV_SOURCES "${SPIRV_DEP_DIR}/*.cpp" "${SPIRV_DEP_DIR}/*.cc")
file(GLOB_RECURSE RENDERING_SOURCES "${CMAKE_SOURCE_DIR}/src/rendering/*.cpp" "${CMAKE_SOURCE_DIR}/src/rendering/*.h" "${CMAKE_SOURCE_DIR}/src/rendering/*.hpp")
file(GLOB_RECURSE COMPILED_SHADERS "${CMAKE_SOURCE_DIR}/shaders/*spv")

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${DEP_DIR}"
    "${Vulkan_INCLUDE_DIRS}"
    "${CATCH2_DEP_DIR}/Catch2"
)

add_library(SPIRV_LIB ${SPIRV_SOURCES})
set_target_properties(SPIRV_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)

add_library(RENDERING_LIB ${RENDERING_SOURCES})
set_target_properties(RENDERING_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)

target_link_libraries(RENDERING_LIB SPIRV_LIB)


add_executable(ASTER_EXE "${CMAKE_SOURCE_DIR}/src/main.cpp")
set_target_properties(ASTER_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_link_libraries(ASTER_EXE ${DEP_LIBRARIES} RENDERING_LIB) 

add_subdirectory("${CMAKE_SOURCE_DIR}/dependency/Catch2")
add_subdirectory("${CMAKE_SOURCE_DIR}/tests/shaderparsing")