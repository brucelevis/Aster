cmake_minimum_required(VERSION 3.19)

file(GLOB_RECURSE MESH_EDITOR_SOURCES "${CMAKE_SOURCE_DIR}/src/application/mesh_editor/*.cpp" 
                                      "${CMAKE_SOURCE_DIR}/src/application/mesh_editor/*.h" 
                                      "${CMAKE_SOURCE_DIR}/src/application/mesh_editor/*.hpp")

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib"
                   "${Vulkan_LIBRARIES}")

add_executable(MESH_EDITOR_EXE ${MESH_EDITOR_SOURCES})
set_target_properties(MESH_EDITOR_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_link_libraries(MESH_EDITOR_EXE ${DEP_LIBRARIES} ENGINE_LIB IMGUI_LIB) 
