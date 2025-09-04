#pragma once

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL false
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"Shader.h"
#include "InputCamera.h"
#include "Options.h"

class Camera
{
public:
	// Stores the main vectors of the camera
	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Up;

	float FOV;
	float nearPlane;
	float farPlane;

	Options options;

	// Prevents the camera from jumping around when first clicking left click
	bool firstClick = true;

	// Stores the width and height of the window
	int width;
	int height;

	// Adjust the speed of the camera and it's sensitivity when looking around
	float speed;
	float sensitivity;

	// Camera constructor to set up initial values
	Camera(Options options, InputCamera startCamera);

	// Updates and exports the camera matrix to the Vertex Shader
	glm::mat4 Matrix(Shader& shader);
	// Handles camera inputs
	void Inputs(GLFWwindow* window);
};