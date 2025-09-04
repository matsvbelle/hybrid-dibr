#include"SSBO.h"

// Constructor that generates a Shader Storage Buffer Object and links it to vertices
SSBO::SSBO()
{
	glGenBuffers(1, &ID);
}

void SSBO::LinkData(const void* data, size_t size, int base)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, base, ID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::GetData(void* data, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Binds the SSBO
void SSBO::Bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
}

// Unbinds the SSBO
void SSBO::Unbind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Deletes the SSBO
void SSBO::Delete()
{
	glDeleteBuffers(1, &ID);
}