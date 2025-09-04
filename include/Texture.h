#pragma once

#include<glad/glad.h>
#include<stb/stb_image.h>

#include "Shader.h"

struct TextureData
{
	GLenum texType;
	GLenum slot;
	GLenum format; 
	GLenum pixelType;
	GLenum internalFormat;
};

struct TexUniform
{
	const char* uniform;
	GLuint unit;
	GLenum slot;
};

class Texture
{
public:
	GLuint ID;
	TextureData textureData;
	int width;
	int height;

	Texture();
	Texture(TextureData textureData, int width, int height);
	// Assigns a texture unit to a texture
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Bind Data to a Texture
	void BindData(const void * data);
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};