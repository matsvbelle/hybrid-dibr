#include "ComputeShader.h"

// Constructor that builds a compute Shader
ComputeShader::ComputeShader(const char* computeFile)
{
	// Read computeFile and store the strings
	std::string computeCode = get_file_contents(shadersPath + computeFile);

	// Convert the shader source strings into character arrays
	const char* computeSource = computeCode.c_str();

	// Create Compute Shader Object and get its reference
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	// Attach Compute Shader source to the Vertex Shader Object
	glShaderSource(computeShader, 1, &computeSource, NULL);
	// Compile the Compute Shader into machine code
	glCompileShader(computeShader);
	// Checks if Shader compiled succesfully
	compileErrors(computeShader, "COMPUTE");

	// Create Shader Program Object and get its reference
	ID = glCreateProgram();
	// Attach the Compute Shader to the Shader Program
	glAttachShader(ID, computeShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(ID);
	// Checks if Shaders linked succesfully
	compileErrors(ID, "PROGRAM");

	// Delete the now useless Compute Shader objects
	glDeleteShader(computeShader);
}

// Activates the Shader Program
void ComputeShader::Activate()
{
	glUseProgram(ID);
}

// Dispatch the Compute Shader and wait for the result
void ComputeShader::Dispatch(int width_x, int width_y, int width_z)
{
	glDispatchCompute(width_x, width_y, width_z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

// Deletes the Shader Program
void ComputeShader::Delete()
{
	glDeleteProgram(ID);
}

// Checks if the different Shaders have compiled properly
void ComputeShader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}