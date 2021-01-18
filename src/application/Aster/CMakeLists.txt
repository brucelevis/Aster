cmake_minimum_required(VERSION 3.19)

file(GLOB_RECURSE ASTER_SOURCES "${CMAKE_SOURCE_DIR}/src/application/Aster/*.cpp" "${CMAKE_SOURCE_DIR}/src/application/Aster/*.h" "${CMAKE_SOURCE_DIR}/src/application/Aster/*.hpp")

set (DEP_LIBRARIES "${GLFW_DEP_DIR}/glfw3.lib"
                   "${Vulkan_LIBRARIES}"
                   "${YAML_CPP_DEP_DIR}/lib/yaml-cppd.lib" 
                   "${KTX_DIR}/lib/ktx.lib")

add_executable(ASTER_EXE ${ASTER_SOURCES})
set_target_properties(ASTER_EXE PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
                        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug")

target_link_libraries(ASTER_EXE ${DEP_LIBRARIES} ENGINE_LIB ECS_LIB) 

add_custom_command(	
        TARGET ASTER_EXE POST_BUILD	
        COMMAND ${CMAKE_COMMAND} -E copy	
                ${KTX_DIR}/dll/ktx.dll	
                ${RUNTIME_OUTPUT_DIRECTORY}/Debug/ktx.dll
        COMMAND ${CMAKE_COMMAND} -E copy	
                ${KTX_DIR}/dll/ktx.dll	
                ${RUNTIME_OUTPUT_DIRECTORY}/Release/ktx.dll)
