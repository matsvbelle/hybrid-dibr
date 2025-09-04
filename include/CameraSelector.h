#pragma once

#include <algorithm>  

#include "InputCamera.h"
#include "Options.h"
#include "thread_pool/thread_pool.h"

class CameraSelector
{
public:
	CameraSelector();

	CameraSelector(std::vector<InputCamera> input_cameras, Options options, dp::thread_pool<>* pool);

	void EnqueueNext(glm::mat4 currentCameraPosition);

	InputCamera textureCamera[3];

	int nrCameras;

private:
	dp::thread_pool<>* pool;
	std::vector<InputCamera> input_cameras;
	Options options;

	bool canEnqueue = true;

	std::future<std::vector<int>> result;
};