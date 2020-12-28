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
set(YAML_CPP_DEP_DIR "${DEP_DIR}/yaml-cpp")
set(STB_IMAGE_DEP_DIR "${DEP_DIR}/stb_image")

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib" "${Vulkan_LIBRARIES}" "${YAML_CPP_DEP_DIR}/lib/yaml-cppd.lib")

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${DEP_DIR}"
    "${Vulkan_INCLUDE_DIRS}"
    "${CATCH2_DEP_DIR}/Catch2"
)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/engine")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependency/Catch2")
add_subdirectory("${CMAKE_SOURCE_DIR}/tests/shader_parsing")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependency/ecs")

add_executable(ASTER_EXE "${CMAKE_SOURCE_DIR}/src/main.cpp")
set_target_properties(ASTER_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_link_libraries(ASTER_EXE ${DEP_LIBRARIES} ENGINE_LIB ECS_LIB) 