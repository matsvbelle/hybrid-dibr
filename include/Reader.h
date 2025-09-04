#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "delaunator/delaunator.h"
#include "SSBO.h"
#include "ComputeShader.h"
#include "Mesh.h"
#include "DynamicMesh.h"
#include "InputCamera.h"
#include "Options.h"
#include "video_reader.hpp"

class Reader {
public:
    Reader(std::string json_file, Options options);
    Reader(std::vector<InputCamera> input_cameras, Options options);

    void parse();

    void parseTextures();

    void exportBinary(std::string binary_file);

    void readBinary(std::string binary_file);

    DynamicMesh createDynamicMesh();

    std::vector<InputCamera> cameras;
private:
    Options options;

    std::vector<std::vector<Vertex>> _allVertices;
    std::vector<std::vector<GLuint>> _allIndices;
};