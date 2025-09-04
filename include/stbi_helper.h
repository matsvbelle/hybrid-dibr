#pragma once

#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <string>


void saveImage(std::string filepath, GLFWwindow* w);

unsigned char* readImage(std::string filepath, int expected_width, int expected_height);

void writeImage(std::string filepath, unsigned char* image, int width, int height, int nrChannels);