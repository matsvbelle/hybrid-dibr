#pragma once

#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

// Structure to standardize the vertices used in the meshes
struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
};

class VBO
{
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	// Constructor that generates a VBO ID
	VBO();

	// Change VBO Data
	void LinkData(std::vector<Vertex>& vertices);
	void LinkNewData(std::vector<Vertex>& vertices);
	// Binds the VBO
	void Bind();
	// Unbinds the VBO
	void Unbind();
	// Deletes the VBO
	void Delete();
};