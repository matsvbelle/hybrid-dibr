#pragma once

#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glad/glad.h>

class RBO
{
public:
	// Reference ID of the Render Buffer Object
	GLuint ID;
	// Constructor that generates a RBO ID
	RBO();

	void CreateDepthStencil(int width, int height);

	// Binds the RBO
	void Bind();
	// Unbinds the RBO
	void Unbind();
	// Deletes the RBO
	void Delete();
};