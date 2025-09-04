#pragma once

#include "log/aixlog.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>

#include "thread_pool/thread_pool.h"

#include "Reader.h"
#include "Options.h"

#include "Shader.h"
#include "Camera.h"
#include "Controller.h"

#include "DynamicMesh.h"
#include "DynamicTexture.h"

#include "DepthMap.h"
#include "CameraSelector.h"

#include "FBO.h"
#include "RBO.h"
#include "Texture.h"

class Application
{
public:
	Application(GLFWwindow* window, Reader& reader, Options options);

    void Run();

    void Shutdown();

private:
    float rectangleVertices[24] =
    {
        // Coords    // texCoords
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,

        1.0f,  1.0f,  1.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    GLFWwindow* window;
    Options options;
    dp::thread_pool<>* pool;

    Shader* shaderProgram;
    Camera* camera;
    Controller* controller;

    CameraSelector cameraSelector;

    DynamicMesh mesh;
    DynamicTexture texture[3];
    DepthMap depthMap[3];

    VAO rectVAO;
    VBO rectVBO;
    Shader* postRenderProgram;
    FBO postRenderFBO;
    RBO postRenderRBO;
    Texture postRenderTexture;

    double prevTime;
    double lastTime;
    int nbFrames;
};