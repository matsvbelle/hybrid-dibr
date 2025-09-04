#include "DynamicMesh.h"

DynamicMesh::DynamicMesh()
{}

DynamicMesh::DynamicMesh(std::vector<std::vector<Vertex>>& verticesVector, std::vector<std::vector<GLuint>>& indicesVector)
{
    DynamicMesh::verticesVector = verticesVector;
    DynamicMesh::indicesVector = indicesVector;
	if (verticesVector.size() != indicesVector.size())
	{
		std::cout << "ERROR: Dynamic Mesh with 2 vectors of different size" << std::endl;
	}

	currentFrame = 0;

    VAO.Bind();

	// Link Vertex Buffer Object to vertices
	VBO.LinkData(verticesVector[currentFrame]);
	// Link Element Buffer Object to indices
	EBO.LinkData(indicesVector[currentFrame]);

	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));

	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void DynamicMesh::AddFrame()
{
	++currentFrame;
	if (currentFrame >= verticesVector.size())
	{
		currentFrame = 0;
	}
	VAO.Bind();
	
	// Link Vertex Buffer Object to vertices
	VBO.LinkNewData(verticesVector[currentFrame]);
	// Link Element Buffer Object to indices
	EBO.LinkNewData(indicesVector[currentFrame]);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));

	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void DynamicMesh::Draw(Shader& shader)
{
	// Bind shader to be able to access uniforms
	shader.Activate();
	VAO.Bind();

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, int(indicesVector[currentFrame].size()), GL_UNSIGNED_INT, 0);
}

void DynamicMesh::Delete()
{
	VAO.Delete();
	VBO.Delete();
	EBO.Delete();
}