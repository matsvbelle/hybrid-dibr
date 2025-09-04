#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>

#include "Options.h"
#include "Reader.h"
#include "Application.h"

GLFWwindow* createGLWindowContext(int width, int height)
{	
	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, "Light Field Renderer", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return NULL;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	int version = gladLoadGL();
    if (version == 0) {
        std::cout << "Failed to initialize OpenGL context." << std::endl;
		glfwTerminate();
        return NULL;
    }
	// Specify the viewport of OpenGL in the Window
	glViewport(0, 0, width, height);

	return window;
}

int main(int argc, char* argv[])
{
	// Parse options
	Options options = Options(argc, argv);

	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/*
	******************************************************
	* STEP 1: PREPROCESSING
	******************************************************
	*/
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		GLFWwindow* invisible_window = createGLWindowContext(options.application_width, options.application_height);
		if (invisible_window == NULL) return -1;

		// Initialize Reader and use it to parse Json files
		Reader reader(options.json_file, options); //"example/Painter/Images/Painter.json"
		if (options.input_binary != "")
		{
			reader.readBinary(options.input_binary);
		}
		else
		{
			reader.parse(); 
		}
		if (options.output_binary != "")
		{
			reader.exportBinary(options.output_binary);
		}

		glfwDestroyWindow(invisible_window);

	/*
	******************************************************
	* STEP 2: RUNNING THE LIVE APPLICATION
	******************************************************
	*/
	if (!options.only_precomputing)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		GLFWwindow* window = createGLWindowContext(options.application_width, options.application_height);
		if (window == NULL) return -1;

		// Initialize Application
		Application application(window, reader, options);

		// Main Application loop
		application.Run();

		// Cleanly exit the Application
		application.Shutdown();

		// Delete window before ending the program
		glfwDestroyWindow(window);
	}

	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}