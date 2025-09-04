#pragma once

#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glad/glad.h>

class SSBO
{
public:
	// Reference ID of the Shader Storage Buffer Object
	GLuint ID;
	// Constructor that generates a SSBO ID
	SSBO();

	void LinkData(const void* data, size_t size, int base);
	void GetData(void* data, size_t size);

	// Binds the SSBO
	void Bind();
	// Unbinds the SSBO
	void Unbind();
	// Deletes the SSBO
	void Delete();
};