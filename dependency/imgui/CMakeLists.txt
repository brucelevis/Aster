file(GLOB_RECURSE IMGUI_SOURCES "${IMGUI_DEP_DIR}/*.cpp" "${IMGUI_DEP_DIR}/*.cc" "${IMGUI_DEP_DIR}/*.h")

add_library(IMGUI_LIB ${IMGUI_SOURCES})
set_target_properties(IMGUI_LIB PROPERTIES 
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED YES
                        CXX_EXTENSIONS NO)
