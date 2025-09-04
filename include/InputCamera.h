#pragma once


#define GLM_ENABLE_EXPERIMENTAL false
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fstream>
#include <iostream>
#include <vector>

#include <nlohmann/json.hpp>


class Intrinsics {
public:
	int res[2] = { 0,0 };  // resolution, in pixels
	glm::vec2 focal = glm::vec2(0, 0); // focal length, in pixels
	glm::vec2 FOV = glm::vec2(0, 0); // field-of-view, in radians
	int frames = 0;

	Intrinsics() {};

	Intrinsics(nlohmann::json params) {
		std::string k = ""; // useful for error handling
		try {
			k = "Resolution";
			res[0] = (int)params[k][0];
			res[1] = (int)params[k][1];
			k = "Focal";
			focal = glm::vec2((float)params[k][0], (float)params[k][1]);
			k = "frames";
			frames = (int)params[k];
		}
		catch (nlohmann::json::exception e) {
			std::cout << "Error: error while parsing key \"" << k << "\" in the JSON file" << std::endl;
			exit(-1);
		}

		FOV = glm::vec2(2.0f * atan(res[0] / 2.0f / focal.x),  2.0f * atan(res[1] / 2.0f / focal.y));
	}
};

class InputCamera {
public:
	std::string jsonDirectory = "";
	std::string nameColor = "";
	std::string nameDepth = "";
	glm::mat4 model = glm::mat4();
	glm::mat4 view = glm::mat4();

	glm::vec2 pp = glm::vec2(0, 0); // principal point, in pixels
	float z_near = 0;  // depth of near plane
	float z_far = 0;   // depth of far plane

	
	Intrinsics intrinsics;

	InputCamera() {}

	InputCamera(nlohmann::json params, Intrinsics intrinsics, std::string jsonDirectory) {
		this->intrinsics = intrinsics;
		this->jsonDirectory = jsonDirectory;
		std::string k = ""; // useful for error handling
		try {
			k = "Principal_point";
			pp = glm::vec2((float)params[k][0], (float)params[k][1]);
			k = "Depth_range";
			z_near = (float)params[k][0];
			z_far = (float)params[k][1];
			k = "NameColor";
			nameColor = params[k].get<std::string>();
			k = "NameDepth";
			nameDepth = params[k].get<std::string>();

			k = "model";
			// glm stores matrices in column-major order
			model = glm::mat4(
				(float)params[k][0][0],(float)params[k][1][0], (float)params[k][2][0], (float)params[k][3][0],
				(float)params[k][0][1],(float)params[k][1][1], (float)params[k][2][1], (float)params[k][3][1],
				(float)params[k][0][2],(float)params[k][1][2], (float)params[k][2][2], (float)params[k][3][2],
				(float)params[k][0][3],(float)params[k][1][3], (float)params[k][2][3], (float)params[k][3][3]
			);
			view = glm::inverse(model);
		}
		catch (nlohmann::json::exception e) {
			std::cout << "Error: error while parsing key \"" << k << "\" in the JSON file";
			if (k != "NameColor") {
				std::cout << " for camera " << params["NameColor"].get<std::string>();
			}
			std::cout << std::endl;
			exit(-1);
		}
	}

	static std::vector<InputCamera> readCameraInputJson(std::string jsonPath) 
	{
		std::vector<InputCamera> cameras;

		// get the directory the json file is in
		size_t pos = jsonPath.find_last_of("\\/");
		std::string jsonDirectory = (std::string::npos == pos) ? "" : jsonPath.substr(0, pos);
		jsonDirectory = jsonDirectory + "/";

		// open the json file
		std::ifstream file;
		file.open(jsonPath, std::ios::in);
		if (!file) {
			std::cout << "Error: failed to open JSON file " << jsonPath << std::endl;
			return cameras;
		}

		// parse json file
		nlohmann::json j;
		try {
			file >> j;
		}
		catch (nlohmann::json::parse_error & e) {
			std::cout << e.what() << std::endl;
			std::cout << "Error: failed to parse JSON file " << jsonPath << ". Check for syntax errors." << std::endl;
			file.close();
			return cameras;
		}
		file.close();

		// read the intrinsics
		Intrinsics intrinsics(j);

		// read the extrinsics
		if (!j.contains("cameras")) {
			std::cout << "Error: the input JSON file should contain a key \"cameras\" with a list of cameras as value." << std::endl;
			return cameras;
		}
		
		for (int i = 0; i < j["cameras"].size(); ++i)
		{
			cameras.push_back(InputCamera(j["cameras"][i], intrinsics, jsonDirectory));
		}

		j.clear();
		return cameras;
	}
};

