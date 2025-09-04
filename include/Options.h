#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Options {
public:
    // Input paths
    std::string json_file;
    std::string input_binary;

    // Output paths
    std::string output_binary;
    std::string output_edges;
    std::string output_color;
    std::string output_depth;
    std::string output_render;
    std::string output_times;

    // View options
    bool noTexture;
    bool noDepthBuffer;
    bool noHoleFilling;
    bool noCameraSelection;
    glm::vec3 background;
    bool cameraUp;
    bool cameraDepth;
    float cameraFastSpeed;
    float cameraPrecisionSpeed;
    float cameraFastSensitivity;
    float cameraPrecisionSensitivity;
    int nrCameras;
    int staticViewCamera;

    // Window options
    int application_width;
    int application_height;

    // Precomputing options
    float edge_cutoff;
    int edge_scaling;
    float triangle_deletion_margin;
    int triangle_straight_margin;
    bool use_colored_edges;
    float inner_position_epsilon;
    float outer_position_epsilon;

    int max_pos_x;
    int min_pos_x;
    int x_partitions;
    int max_pos_y;
    int min_pos_y;
    int y_partitions;
    int max_pos_z;
    int min_pos_z;
    int z_partitions;

    // Frame Options
    int startFrame;
    int limitFrames;
    std::vector<int> cameras;

    bool noVsync;
    int thread_pool_size;
    int texture_lookahead;
    float textureBias;
    int x_reach;
    int y_reach;
    int camera_grid_x_partitions;
    int camera_grid_y_partitions;
    float camera_grid_width;
    float camera_grid_height;
    float camera_grid_distance;

    // Generic options
    bool only_precomputing;
    bool singleFrame;
public:
    Options();
	Options(int argc, char* argv[]);
};