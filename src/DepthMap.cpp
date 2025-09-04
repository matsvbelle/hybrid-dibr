#include "DepthMap.h"

DepthMap::DepthMap()
{}

DepthMap::DepthMap(Shader& shader, int width, int height, TexUniform tex_uniform)
{
	DepthMap::width = width;
	DepthMap::height = height;

	depthMapProgram = new Shader("depthMapVertex.glsl", "depthMapFragment.glsl");
	depthMapProgram->Activate();

	depthMapTexture = Texture(TextureData{GL_TEXTURE_2D, tex_uniform.slot, GL_RGB, GL_FLOAT, GL_RGB}, width, height);
	depthMapFBO.Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMapTexture.ID, 0);

	depthMapRBO.Bind();
	depthMapRBO.CreateDepthStencil(width, height);

	GLenum depthMapFBOStatus = depthMapFBO.Status();
	if (depthMapFBOStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Depth-mapping FBO error: " << depthMapFBOStatus << std::endl;
	depthMapFBO.Unbind();

	shader.Activate();
	depthMapTexture.texUnit(shader, tex_uniform.uniform, tex_uniform.unit);
}

void DepthMap::Export(InputCamera& camera)
{
	depthMapProgram->Activate();
	glUniformMatrix4fv(glGetUniformLocation(depthMapProgram->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.model));
	glUniform2fv(glGetUniformLocation(depthMapProgram->ID, "pp"), 1, glm::value_ptr(camera.pp));
	glUniform2fv(glGetUniformLocation(depthMapProgram->ID, "focal"), 1, glm::value_ptr(camera.intrinsics.focal));
	glUniform2f(glGetUniformLocation(depthMapProgram->ID, "res"), float(camera.intrinsics.res[0]), float(camera.intrinsics.res[1]));
	glUniform1f(glGetUniformLocation(depthMapProgram->ID, "z_far"), camera.z_far);
	glUniform1f(glGetUniformLocation(depthMapProgram->ID, "z_near"), camera.z_near);
}

void DepthMap::Draw(DynamicMesh& mesh)
{
	depthMapProgram->Activate();
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	depthMapFBO.Bind();

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mesh.Draw(*depthMapProgram);

	depthMapFBO.Unbind();
}

void DepthMap::BindTexture()
{
	depthMapTexture.Bind();
}

void DepthMap::UnbindTexture()
{
	depthMapTexture.Unbind();
}

void DepthMap::Delete()
{
	depthMapProgram->Delete();
	depthMapTexture.Delete();
	depthMapRBO.Delete();
	depthMapFBO.Delete();
	delete depthMapProgram;
}