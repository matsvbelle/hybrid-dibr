#include "CameraSelector.h"

CameraSelector::CameraSelector()
{}

CameraSelector::CameraSelector(std::vector<InputCamera> input_cameras, Options options, dp::thread_pool<>* pool)
{
	if (options.cameras.empty())
	{
		for (int i = 0; i < input_cameras.size(); ++i)
		{
			options.cameras.push_back(i);
		}
	}

	nrCameras = options.nrCameras;
	if (options.cameras.size() < options.nrCameras)
	{
		nrCameras = int(options.cameras.size());
	}

	textureCamera[0] = input_cameras[options.cameras[0]];
	if (nrCameras > 1) textureCamera[1] = input_cameras[options.cameras[1]];
	if (nrCameras > 2) textureCamera[2] = input_cameras[options.cameras[2]];

	CameraSelector::input_cameras = input_cameras;
	CameraSelector::options = options;
	CameraSelector::pool = pool;
}

std::vector<int> calculateCameras(std::vector<InputCamera> input_cameras, Options options, int nrCameras, glm::mat4 currentCameraPosition)
{
	int x_partitions = options.camera_grid_x_partitions;
	int y_partitions = options.camera_grid_y_partitions;
	float width = options.camera_grid_width;
	float height = options.camera_grid_height;
	float distanceFromCamera = options.camera_grid_distance;

	std::vector<std::vector<std::vector<int>>> points(x_partitions, std::vector<std::vector<int>>(y_partitions));

	for (int x = 0; x < x_partitions; ++x)
	{
		for (int y = 0; y < y_partitions; ++y)
		{
			glm::vec2 uv = glm::vec2(x/float(x_partitions), y/float(y_partitions));

			glm::vec2 screenPos = glm::vec2((uv.x - 0.5) * width, (uv.y - 0.5) * height);

			glm::vec3 cameraRight   = glm::vec3(currentCameraPosition[0]);
			glm::vec3 cameraUp      = glm::vec3(currentCameraPosition[1]);
			glm::vec3 cameraForward = glm::vec3(currentCameraPosition[2]);
			glm::vec3 cameraOrigin  = glm::vec3(currentCameraPosition[3]);

			glm::vec3 pos = cameraOrigin + screenPos.x * cameraRight + screenPos.y * cameraUp + distanceFromCamera * cameraForward;

			for (int index = 0; index < options.cameras.size(); ++index)
			{
				int cameraNr = options.cameras[index];
				InputCamera camera = input_cameras[cameraNr];

				glm::vec4 cameraPosition = glm::inverse(camera.model) * glm::vec4(-pos.x, -pos.y, pos.z, 1.0);
				cameraPosition /= cameraPosition.w;

				if (cameraPosition.z < 0)
				{
					continue;
				}

				float img_x = float(camera.intrinsics.focal.x * (cameraPosition.x / cameraPosition.z) + camera.pp.x - 0.5);
				float img_y = float(camera.intrinsics.focal.y * (cameraPosition.y / cameraPosition.z) + camera.pp.y - 0.5);

				if (img_x < 0 || img_x > camera.intrinsics.res[0] || img_y < 0 || img_y > camera.intrinsics.res[1])
				{
					continue;
				}

				points[x][y].push_back(index);
			}
		}
	}

	std::vector<int> result = std::vector<int>(nrCameras);
	for (int i = 0; i < nrCameras; ++i)
	{
		std::vector<int> counts = std::vector<int>(options.cameras.size());

		for (int x = 0; x < x_partitions; ++x)
		{
			for (int y = 0; y < y_partitions; ++y)
			{
				bool valid = true;
				for (int j = 0; j < i; ++j)
				{
					for (int index = 0; index < points[x][y].size(); ++index)
					{
						if (points[x][y][index] == result[j])
						{
							valid = false;
						}
					}
				}
				if (valid)
				{
					for (int index = 0; index < points[x][y].size(); ++index)
					{
						counts[points[x][y][index]] += 1;
					}
				}
			}
		}

		int maxCount = -1;
		int maxIndex = i;
		for (int index = 0; index < options.cameras.size(); ++index)
		{
			if (counts[index] > maxCount)
			{
				bool alreadyUsedCamera = false;
				for (int j = 0; j < i; ++j)
				{
					if (result[j] == index)
					{
						alreadyUsedCamera = true;
					}
				}
				if (!alreadyUsedCamera)
				{
					maxCount = counts[index];
					maxIndex = index;
				}
			}
		}
		result[i] = maxIndex;
	}

	std::sort(std::begin(result), std::end(result));

	return result;
}

void CameraSelector::EnqueueNext(glm::mat4 currentCameraPosition)
{
	if (result.valid() && result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		std::vector<int> values = result.get();
		for (int i = 0; i < nrCameras; ++i)
		{
			//std::cout << i << ": " << options.cameras[values[i]] << std::endl;
			textureCamera[i] = input_cameras[options.cameras[values[i]]];
		}
		canEnqueue = true;
	}

	if (canEnqueue)
	{
		result = pool->enqueue([](std::vector<InputCamera> input_cameras, Options options, int nrCameras, glm::mat4 currentCameraPosition) -> std::vector<int> { 
			return calculateCameras(input_cameras, options, nrCameras, currentCameraPosition);
		}, input_cameras, options, nrCameras, currentCameraPosition);

		canEnqueue = false;
	}
}