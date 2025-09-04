#include "Controller.h"

#include "stbi_helper.h"


Controller::Controller()
{}

void Controller::Export(Shader& shader, GLFWwindow* window, Options options)
{
	if (viewMode == GL_FILL)
	{
		glUniform1i(glGetUniformLocation(shader.ID, "fillHoles"), !options.noHoleFilling);
	}
	else
	{
		glUniform1i(glGetUniformLocation(shader.ID, "fillHoles"), false);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || options.singleFrame)
	{
		if (savePicturePressDelay == 0)
		{
			saveImage(options.output_render, window);
		}
	}
	if (savePicturePressDelay != 0) 
	{
		savePicturePressDelay--;
	}
}

void Controller::Inputs(GLFWwindow* window)
{
	glPolygonMode(GL_FRONT_AND_BACK, viewMode);

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{	
		viewMode = GL_FILL;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{	
		viewMode = GL_POINT;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{	
		viewMode = GL_LINE;
	}
}
