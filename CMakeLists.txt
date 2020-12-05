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

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib" "${Vulkan_LIBRARIES}")

file(GLOB_RECURSE SPIRV_SOURCES "${SPIRV_DEP_DIR}/*.cpp" "${SPIRV_DEP_DIR}/*.cc")
file(GLOB_RECURSE ASTER_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/src/*.hpp")
file(GLOB_RECURSE COMPILED_SHADERS "${CMAKE_SOURCE_DIR}/shaders/*spv")

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${DEP_DIR}"
    "${Vulkan_INCLUDE_DIRS}"
)

add_library(SPIRV_LIB ${SPIRV_SOURCES})
set_target_properties(SPIRV_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)

add_executable(ASTER_EXE)

set_target_properties(ASTER_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_sources(ASTER_EXE PRIVATE ${ASTER_SOURCES})

target_link_libraries(ASTER_EXE ${DEP_LIBRARIES} SPIRV_LIB) 

