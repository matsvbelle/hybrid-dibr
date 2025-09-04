#pragma once

#define GLM_ENABLE_EXPERIMENTAL false
#include <queue>
#include <future>
#include <utility>
#include <ranges>
#include <glm/glm.hpp>

#include "thread_pool/thread_pool.h"
#include "video_reader.hpp"
#include "Shader.h"
#include "InputCamera.h"
#include "Options.h"
#include "Texture.h"

struct ThreadState
{
	VideoReaderState vr_state;
	int currentFrame;
	int nrFrames;
	InputCamera textureCamera;
	std::vector<int> i_frames;
};

class DynamicTexture
{
public:
	std::future<ThreadState> future_result;
	dp::thread_pool<>* pool;

	Options options;
	InputCamera textureCamera;
	bool static_texture;

	Texture luminanceTexture;
	Texture chrominanceTexture;

	DynamicTexture();

	// Initializes the mesh
	DynamicTexture(Shader& shader, Options options, dp::thread_pool<>* pool, InputCamera textureCamera, TexUniform luminanceUniform, TexUniform chrominanceUniform);
	// Start decoding next camera frame
	void EnqueueFutureResult(ThreadState thread_state, InputCamera textureCamera, Options options);
	// Add a frame to the textures
	void AddFrame(InputCamera newTextureCamera);
	// Export current camera info
	void Export(Shader& shader, int i);
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};