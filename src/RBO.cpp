#include "RBO.h"

// Constructor that generates a Render Buffer Object
RBO::RBO()
{
	glGenRenderbuffers(1, &ID);
}

void RBO::CreateDepthStencil(int width, int height)
{
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ID);
}

// Binds the RBO
void RBO::Bind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, ID);
}

// Unbinds the RBO
void RBO::Unbind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// Deletes the RBO
void RBO::Delete()
{
	glDeleteRenderbuffers(1, &ID);
}