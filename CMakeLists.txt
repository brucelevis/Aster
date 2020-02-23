cmake_minimum_required(VERSION 3.15)
project(Aster)
set(CMAKE_CXX_STANDARD 17)

include_directories(thirdparty/include)
LINK_DIRECTORIES(thirdparty/lib/glfw)

add_executable(Aster
		main.cpp
		#thirdparty
		thirdparty/src/glad/glad.c)

TARGET_LINK_LIBRARIES(Aster
					glfw3
					opengl32)