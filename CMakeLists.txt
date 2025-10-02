# Default Paphos project starter
# flecs/PyTorch with AOT Inductor pipeline

cmake_minimum_required(VERSION 3.18)
project(project)

set(DEBUG_FLAGS "-g -O1 -pthread -pedantic -fpermissive")
set(RELEASE_FLAGS "-O3 -pthread -pedantic -fpermissive")
set(CMAKE_BUILD_TYPE Release)

include_directories("src")
include_directories("deps")
include_directories("deps/freeglut-3.4.0/include")
include_directories("deps/glfw/include")
include_directories("deps/glfw/deps")
include_directories("deps/nanovg/src")
include_directories("deps/flecs-4.1.1/include")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build Flecs as a shared library" FORCE)
include_directories("deps/libevdev-master/include")
include_directories("deps/libevdev-master")
# include_directories("deps/libvncserver-0.9.15/include")

set(GLAD_GL "deps/glfw/deps/glad/gl.h")

add_subdirectory("deps/flecs-4.1.1")
# add_subdirectory("deps/libvncserver-0.9.15")
add_subdirectory("deps/freeglut-3.4.0" EXCLUDE_FROM_ALL)
add_subdirectory("deps/glfw" EXCLUDE_FROM_ALL)
add_subdirectory("deps/nanovg/cmake-nanovg" EXCLUDE_FROM_ALL)

find_package(LibVNCServer)
find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp ${GLAD_GL})

target_compile_options(${PROJECT_NAME} PUBLIC -w)
target_link_libraries(${PROJECT_NAME} flecs glfw nanovg freeglut evdev X11 Xi Xtst SDL2)

if(LibVNCServer_FOUND)
    target_link_libraries(${PROJECT_NAME} LibVNCServer::vncclient)
    target_link_libraries(${PROJECT_NAME} LibVNCServer::vncserver)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
