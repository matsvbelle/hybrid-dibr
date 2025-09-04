#pragma once

#include<glad/glad.h>

#include "Shader.h"
#include "InputCamera.h"
#include "DynamicMesh.h"
#include "FBO.h"
#include "RBO.h"
#include "Texture.h"

class DepthMap
{
public:
	DepthMap();

	DepthMap(Shader& shader, int width, int height, TexUniform tex_uniform);

	void Export(InputCamera& camera);

	void Draw(DynamicMesh& mesh);

	void BindTexture();

	void UnbindTexture();

	void Delete();

private:
	int width;
	int height;

	Shader* depthMapProgram;
    FBO depthMapFBO;
    RBO depthMapRBO;
    Texture depthMapTexture;
};