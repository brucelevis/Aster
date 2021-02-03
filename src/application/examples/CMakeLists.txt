cmake_minimum_required(VERSION 3.19)

file(GLOB_RECURSE EXAMPLES_SOURCES "${CMAKE_SOURCE_DIR}/src/application/examples/*.cpp" 
                                      "${CMAKE_SOURCE_DIR}/src/application/examples/*.h" 
                                      "${CMAKE_SOURCE_DIR}/src/application/examples/*.hpp")

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib"
                   "${Vulkan_LIBRARIES}")

add_executable(EXAMPLES_EXE ${EXAMPLES_SOURCES})
set_target_properties(EXAMPLES_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_link_libraries(EXAMPLES_EXE ${DEP_LIBRARIES} ENGINE_LIB IMGUI_LIB) 
