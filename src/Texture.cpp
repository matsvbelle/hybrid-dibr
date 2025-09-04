#include "Texture.h"

Texture::Texture()
{}

Texture::Texture(TextureData textureData, int width, int height)
{
	// Assigns the type of the texture ot the texture object
	Texture::textureData = textureData;
	Texture::width = width;
	Texture::height = height;

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);
	glActiveTexture(textureData.slot);
	glBindTexture(textureData.texType, ID);

	// Creates the texture
	glTexImage2D(textureData.texType, 0, textureData.internalFormat, width, height, 0, textureData.format, textureData.pixelType, NULL);

	glTexParameteri(textureData.texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(textureData.texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(textureData.texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(textureData.texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(textureData.texType, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::BindData(const void * data)
{
	Bind();
	glTexImage2D(textureData.texType, 0, textureData.internalFormat, width, height, 0, textureData.format, textureData.pixelType, data);
	Unbind();
}

void Texture::Bind()
{
	glActiveTexture(textureData.slot);
	glBindTexture(textureData.texType, ID);
}

void Texture::Unbind()
{
	glActiveTexture(textureData.slot);
	glBindTexture(textureData.texType, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}