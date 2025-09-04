#pragma once

#include "Shader.h"
#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>


std::string get_file_contents(std::string filename);

class ComputeShader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	ComputeShader(const char* computeFile);

	// Activates the Shader Program
	void Activate();
	// Dispatch the Compute Shader and wait for the result
	void Dispatch(int width_x, int width_y, int width_z);
	// Deletes the Shader Program
	void Delete();
private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);

	// From CMAKE preprocessor
	std::string cmakelists_dir = CMAKELISTS_SOURCE_DIR;
	const std::string shadersPath = cmakelists_dir + "/src/shaders/";
};