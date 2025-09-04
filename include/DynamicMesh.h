#pragma once

#include <string>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "InputCamera.h"
#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include "stbi_helper.h"

class DynamicMesh
{
public:
	std::vector<std::vector<Vertex>> verticesVector;
	std::vector<std::vector<GLuint>> indicesVector;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;
	VBO VBO;
	EBO EBO;

	int currentFrame = 0;

	DynamicMesh();

	// Initializes the mesh
	DynamicMesh(std::vector<std::vector<Vertex>>& verticesVector, std::vector<std::vector<GLuint>>& indicesVector);

	void AddFrame();

    // Draws the mesh
	void Draw(Shader& shader);

	void Delete();
};