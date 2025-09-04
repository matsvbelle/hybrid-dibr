#include"FBO.h"

// Constructor that generates a Shader Storage Buffer Object and links it to vertices
FBO::FBO()
{
	glGenFramebuffers(1, &ID);
}

// Binds the FBO
void FBO::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

// Unbinds the FBO
void FBO::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLenum FBO::Status()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

// Deletes the FBO
void FBO::Delete()
{
	glDeleteFramebuffers(1, &ID);
}