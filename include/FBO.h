#pragma once

#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glad/glad.h>

class FBO
{
public:
	// Reference ID of the Frame Buffer Object
	GLuint ID;
	// Constructor that generates a FBO ID
	FBO();

	GLenum Status();

	// Binds the FBO
	void Bind();
	// Unbinds the FBO
	void Unbind();
	// Deletes the FBO
	void Delete();
};