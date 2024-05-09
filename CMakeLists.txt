cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

project(RunnerEngineRaylib)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#Includes
set(FFMPEG_INSTALL_DIR "/home/kiviniemi/repos/ffmpeg")
set(RAYLIB_INSTALL_DIR "/home/kiviniemi/repos/raylib")
include_directories(
    ${FFMPEG_INSTALL_DIR}/include
    ${RAYLIB_INSTALL_DIR}/src
    ${PROJECT_SOURCE_DIR}/src
    ${ASSIMP_INCLUDE_DIR}
)

#Executable
add_executable(PongEngine src/PongEngine.cpp src/FrameEncoder.cpp)
 
#Link stuff
link_directories(${FFMPEG_INSTALL_DIR}/lib)
link_directories(${RAYLIB_INSTALL_DIR}/src)

file(GLOB FFMPEG_LIBRARIES ${FFMPEG_INSTALL_DIR}/lib/*.a)
file(GLOB RAYLIB_LIBRARIES ${RAYLIB_INSTALL_DIR}/src/*.a)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
target_link_libraries(PongEngine
    OpenGL
    glfw
    ${FFMPEG_LIBRARIES}
    ${RAYLIB_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    pthread 
    dl      
    rt      
    m       
)