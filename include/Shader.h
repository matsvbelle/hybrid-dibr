#pragma once

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>


std::string get_file_contents(std::string filename);

class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	// Constructor that build the Shader Program from 3 or 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile);
	Shader(const char* vertexFile, const char* fragmentFile);
	Shader(const char* computeFile);

	// Activates the Shader Program
	void Activate();
	// Deletes the Shader Program
	void Delete();
private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);

	// From CMAKE preprocessor
	std::string cmakelists_dir = CMAKELISTS_SOURCE_DIR;
	const std::string shadersPath = cmakelists_dir + "/src/shaders/";
};