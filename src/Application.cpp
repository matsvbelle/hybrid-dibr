#include "Application.h"

#include <iostream>
#include "stbi_helper.h"

Application::Application(GLFWwindow* window, Reader& reader, Options options)
{   
    Application::window = window;

    // Generates Shader object using shaders
	shaderProgram = new Shader("renderVertex.glsl", "renderFragment.glsl");
	postRenderProgram = new Shader("postRenderVertex.glsl", "postRenderFragment.glsl");

	postRenderProgram->Activate();
	rectVAO.Bind();
	rectVBO.Bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	rectVBO.Unbind();
	rectVAO.LinkAttrib(rectVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
	rectVAO.LinkAttrib(rectVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	rectVAO.Unbind();

	postRenderTexture = Texture(TextureData{GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_FLOAT, GL_RGB}, options.application_width, options.application_height);
	postRenderFBO.Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postRenderTexture.ID, 0);

	postRenderRBO.Bind();
	postRenderRBO.CreateDepthStencil(options.application_width, options.application_height);

	GLenum postRenderFBOStatus = postRenderFBO.Status();
	if (postRenderFBOStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Depth-mapping FBO error: " << postRenderFBOStatus << std::endl;

	postRenderTexture.texUnit(*postRenderProgram, "screenTexture", 0);

	glUniform2f(glGetUniformLocation(postRenderProgram->ID, "res"), float(options.application_width), float(options.application_height));
	glUniform1i(glGetUniformLocation(postRenderProgram->ID, "x_reach"), options.x_reach);
	glUniform1i(glGetUniformLocation(postRenderProgram->ID, "y_reach"), options.y_reach);
	glUniform3fv(glGetUniformLocation(postRenderProgram->ID, "background"), 1, glm::value_ptr(options.background));
	postRenderFBO.Unbind();

	pool = new dp::thread_pool(options.thread_pool_size);

	cameraSelector = CameraSelector(reader.cameras, options, pool);

	mesh = reader.createDynamicMesh();
	if (!options.noTexture)
	{
		shaderProgram->Activate();
		for (unsigned int i = 0; i < (unsigned int)cameraSelector.nrCameras; ++i)
		{
			std::string luminanceName = "luminanceSampler"+std::to_string(i+1);
			std::string chromaName = "chromaSampler"+std::to_string(i+1);
			TexUniform luminanceUniform{luminanceName.c_str(), i*3, GL_TEXTURE0 + i*3};
			TexUniform chromaUniform{chromaName.c_str(), 1+i*3, GL_TEXTURE1 + i*3};
			texture[i] = DynamicTexture(*shaderProgram, options, pool, cameraSelector.textureCamera[i], luminanceUniform, chromaUniform);

			if (!options.noDepthBuffer)
			{
				std::string depthName = "depthSampler"+std::to_string(i+1);
				TexUniform depthUniform{depthName.c_str(), 2+i*3, GL_TEXTURE2 + i*3};
				depthMap[i] = DepthMap(*shaderProgram, cameraSelector.textureCamera[i].intrinsics.res[0], cameraSelector.textureCamera[i].intrinsics.res[1], depthUniform);
			}
		}
	}
	shaderProgram->Activate();
	glUniform1i(glGetUniformLocation(shaderProgram->ID, "sampleColors"), !options.noTexture);
	glUniform1i(glGetUniformLocation(shaderProgram->ID, "sampleDepth"), !options.noDepthBuffer);
	glUniform1i(glGetUniformLocation(shaderProgram->ID, "nrTextures"), cameraSelector.nrCameras);
	glUniform1f(glGetUniformLocation(shaderProgram->ID, "bias"), options.textureBias);

	if (options.staticViewCamera >= 0 && options.staticViewCamera < reader.cameras.size())
	{
		glUniform1i(glGetUniformLocation(shaderProgram->ID, "staticView"), true);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.model"), 1, GL_FALSE, glm::value_ptr(reader.cameras[options.staticViewCamera].model));
		glUniform2fv(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.pp"), 1, glm::value_ptr(reader.cameras[options.staticViewCamera].pp));
		glUniform2fv(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.focal"), 1, glm::value_ptr(reader.cameras[options.staticViewCamera].intrinsics.focal));
		glUniform2f(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.res"), float(reader.cameras[options.staticViewCamera].intrinsics.res[0]), float(reader.cameras[options.staticViewCamera].intrinsics.res[1]));
		glUniform1f(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.z_far"), reader.cameras[options.staticViewCamera].z_far);
		glUniform1f(glGetUniformLocation(shaderProgram->ID, "staticViewCamera.z_near"), reader.cameras[options.staticViewCamera].z_near);
	}
	else
	{
		glUniform1i(glGetUniformLocation(shaderProgram->ID, "staticView"), false);
	}

	camera = new Camera(options, cameraSelector.textureCamera[0]);
    controller = new Controller();

	prevTime = glfwGetTime();
	lastTime = glfwGetTime();
	nbFrames = 0;

	Application::options = options;

	if (options.noVsync)
	{
		glfwSwapInterval(0);
	}

	if (options.output_times != "")
	{
		AixLog::Log::init<AixLog::SinkFile>(AixLog::Severity::trace, options.output_times);
	}
	if (options.output_times != "") LOG(INFO) << "Starting Render: " << std::endl;
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
	{
		double crntTime = glfwGetTime();
		double delta = crntTime - lastTime;
		nbFrames++;
		if ( delta >= 1.0){
			int framerate{ std::max(1, int(nbFrames / delta)) };
			std::stringstream title;
			title << delta/double(nbFrames) << " ms/frame, " << "Running at " << framerate << " fps.";
			glfwSetWindowTitle(window, title.str().c_str());
			nbFrames = 0;
			lastTime += 1.0;
     	}

		bool newTextures = false;
		if ((crntTime - prevTime) >= 1.0f/30.0f)
		{	
			if (!texture[0].static_texture || options.noTexture)
			{
				newTextures = true;
				double startTime = glfwGetTime();
				mesh.AddFrame();
				if (options.output_times != "") 
				{
					LOG(INFO) << "Vertex size: " << mesh.verticesVector[mesh.currentFrame].size() << std::endl;
					LOG(INFO) << "Indices size: " << mesh.indicesVector[mesh.currentFrame].size() << std::endl;
					LOG(INFO) << "Mesh change time: " << glfwGetTime() - startTime << std::endl;
				}
				if (!options.noTexture)
				{	
					for (int i = 0; i < cameraSelector.nrCameras; ++i)
					{
						startTime = glfwGetTime();
						texture[i].AddFrame(cameraSelector.textureCamera[i]);
						if (options.output_times != "") LOG(INFO) << "Texture change time: " << glfwGetTime() - startTime << std::endl;
					}
				}
			}
			prevTime = crntTime;
		}
		
		if (!options.noTexture && !options.noDepthBuffer)
		{
			for (int i = 0; i < cameraSelector.nrCameras; ++i)
			{
				double startTime = glfwGetTime();
				depthMap[i].Export(texture[i].textureCamera);
				depthMap[i].Draw(mesh);
				if (options.output_times != "") LOG(INFO) << "Depthmap draw time: " << glfwGetTime() - startTime << std::endl;
			}
		}

		double startTime = glfwGetTime();
		shaderProgram->Activate();
		glViewport(0, 0, options.application_width, options.application_height);
		glEnable(GL_DEPTH_TEST);

		postRenderFBO.Bind();

		glClearDepth(1.0f);
		glClearColor(options.background.x, options.background.y, options.background.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		if (!options.noTexture)
		{
			for (int i = 0; i < cameraSelector.nrCameras; ++i)
			{
				texture[i].Export(*shaderProgram, i);
				texture[i].Bind();
				if (!options.noDepthBuffer)
				{
					depthMap[i].BindTexture();
				}
			}
		}

		camera->Inputs(window);
		glm::mat4 cameraMatrix = camera->Matrix(*shaderProgram);
		controller->Inputs(window);

		mesh.Draw(*shaderProgram);

		if (!options.noTexture)
		{
			for (int i = 0; i < cameraSelector.nrCameras; ++i)
			{
				texture[i].Unbind();
				if (!options.noDepthBuffer)
				{
					depthMap[i].UnbindTexture();
				}
			}
		}
		if (options.output_times != "") LOG(INFO) << "Render mesh time: " << glfwGetTime() - startTime << std::endl;

		if (!options.noCameraSelection && newTextures && !options.noTexture)
		{
			startTime = glfwGetTime();
			cameraSelector.EnqueueNext(cameraMatrix);
			if (options.output_times != "") LOG(INFO) << "Camera selection enqueue time: " << glfwGetTime() - startTime << std::endl;
		}

		startTime = glfwGetTime();
		postRenderFBO.Unbind();

		glViewport(0, 0, options.application_width, options.application_height);
		postRenderProgram->Activate();
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		controller->Export(*postRenderProgram, window, options);
		glActiveTexture(GL_TEXTURE0);
		postRenderTexture.Bind();
		rectVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		rectVAO.Unbind();
		postRenderTexture.Unbind();
		if (options.output_times != "") LOG(INFO) << "Postrender time: " << glfwGetTime() - startTime << std::endl;

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();

		if (options.singleFrame)
		{
			controller->Export(*postRenderProgram, window, options);
			break;
		}
		if (options.output_times != "") 
		{
			if (newTextures) LOG(INFO) << "NEW";
			LOG(INFO) << "Frame render time: "<< glfwGetTime() - crntTime << std::endl;
		}
	}
}

void Application::Shutdown()
{
    // Delete all the objects we've created
	for (int i = 0; i < cameraSelector.nrCameras; ++i)
	{
		texture[i].Delete();
		depthMap[i].Delete();
	}
	shaderProgram->Delete();
	postRenderProgram->Delete();
	postRenderTexture.Delete();
	postRenderRBO.Delete();
	postRenderFBO.Delete();
	mesh.Delete();
	rectVAO.Delete();
	rectVBO.Delete();
	delete postRenderProgram;
    delete shaderProgram;
    delete camera;
    delete controller;
	delete pool;
}