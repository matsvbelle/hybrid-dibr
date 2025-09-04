#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <nlohmann/json.hpp>
#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Options.h"


class Controller
{
public:
    // Controller constructor to set up initial values
	Controller();
    // Updates and exports the controller changes to the shader program
	void Export(Shader& shader, GLFWwindow* window, Options options);
	// Handles controller inputs
	void Inputs(GLFWwindow* window);

	GLenum viewMode = GL_FILL;

	int savePicturePressDelay = 0;
};