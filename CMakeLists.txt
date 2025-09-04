cmake_minimum_required(VERSION 3.7)
project(HybridDIBR)

set(CMAKE_CXX_STANDARD 20)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

#include headers
include_directories(${INCLUDE_DIR})

# Add OpenGL
find_package(OpenGL REQUIRED)

# Add Glad
add_library(glad src/glad.c)

# Add GLFW as a submodule (since it has its own CMake files)
add_subdirectory(${LIB_DIR}/glfw)
include_directories(${LIB_DIR}/glfw/include)
add_compile_definitions(GLFW_INCLUDE_NONE)

# Add GLM
include_directories(${LIB_DIR}/glm)

# Add FFmpeg
include_directories(${INCLUDE_DIR}/ffmpeg/include)
Set(AVCODEC_LIB ${INCLUDE_DIR}/FFmpeg/lib/avcodec.lib)
Set(AVFORMAT_LIB ${INCLUDE_DIR}/FFmpeg/lib/avformat.lib)
Set(AVUTIL_LIB ${INCLUDE_DIR}/FFmpeg/lib/avutil.lib)
Set(FFMPEG_DLL_DIR ${INCLUDE_DIR}/FFmpeg/lib/)

# Define the paths to the source files
set(APP_SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Options.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/ComputeShader.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Controller.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/DynamicMesh.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Texture.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/DynamicTexture.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/DepthMap.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraSelector.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/EBO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/VAO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/VBO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/SSBO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/FBO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/RBO.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/Reader.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/stb.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/stbi_helper.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/video_reader.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/delaunator/delaunator.cpp
)

# Define the paths to the shader files
set(APP_SHADERS
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/edgeScaleCompute.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/pointCloudCompute.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/depthMapFragment.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/depthMapVertex.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/postRenderFragment.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/postRenderVertex.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/renderFragment.glsl
 ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/renderVertex.glsl
)

# Give them a name
source_group( "sources" FILES ${APP_SOURCES} )
source_group( "shaders" FILES ${APP_SHADERS} )

# Use the source files for the project
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_SHADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} glfw glad ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${CMAKE_DL_LIBS})

# Define the string "CMAKELISTS_SOURCE_DIR"
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKELISTS_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# add ffmpeg dlls to output folder
file(GLOB FFMPEG_DLLS "${FFMPEG_DLL_DIR}/*.dll")

foreach(dll ${FFMPEG_DLLS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying ${dll} to output directory"
    )
endforeach()