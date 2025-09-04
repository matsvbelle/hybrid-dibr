#include "stbi_helper.h"

void saveImage(std::string filepath, GLFWwindow* w) 
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filepath.c_str(), width, height, nrChannels, buffer.data(), stride);
}

unsigned char* readImage(std::string filepath, int expected_width, int expected_height) 
{
	int width, height, n;
	int forceChannels = 3;
	return stbi_load(filepath.c_str(), &width, &height, &n, forceChannels);
}

void writeImage(std::string filepath, unsigned char* image, int width, int height, int nrChannels)
{
	GLsizei stride = nrChannels * width;
	stbi_write_png(filepath.c_str(), width, height, nrChannels, image, stride);
}