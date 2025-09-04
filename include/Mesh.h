#pragma once

#include <string>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include "stbi_helper.h"


class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	Mesh();

	// Initializes the mesh
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

    // Draws the mesh
	void Draw(Shader& shader, Camera& camera);
};