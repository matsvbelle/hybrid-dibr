#include "Options.h"

#include <iostream>

#include "cxxopts/cxxopts.hpp"

Options::Options()
{}

Options::Options(int argc, char* argv[])
{
    cxxopts::Options options("LightFieldRenderer.exe", "A real-time depth-image-based renderer");
		options.add_options()
			("h,help", "Print help")
			;
		options.add_options("Input Files")
			("json-file", "Path to the .json file with the input light field camera parameters. In the same folder are the .png images per frame.", cxxopts::value<std::string>())
			("b,input-binary", "An optional precomputed binary file for immediate loading and skipping precomputed parsing from the input-json.", cxxopts::value<std::string>()->default_value(""))
            ;
        options.add_options("Output Files")
			("o,output-binary", "An optional binary file path to write the parsed data to for faster loading in the future.", cxxopts::value<std::string>()->default_value(""))
            ("output-edge-images", "An optional folder path to write sobel edges maps to.", cxxopts::value<std::string>()->default_value(""))
            ("output-color-images", "An optional folder path to write colored frames to.", cxxopts::value<std::string>()->default_value(""))
            ("output-depth-images", "An optional folder path to write depth frames to.", cxxopts::value<std::string>()->default_value(""))
            ("output-render", "The png file the render will be written to when E is pressed.", cxxopts::value<std::string>()->default_value("render.png"))
            ("output-times", "An optional file to write time information to.", cxxopts::value<std::string>()->default_value(""))
            ;
		options.add_options("View Options")
			("no-texture", "Rendering the light field as a raw vertex data without showing detailed textures.")
            ("no-depth-buffer", "Rendering the light field texture without checking whats near and far to the cameras.")
            ("no-hole-filling", "Rendering the light field just from DIBR without filling the holes afterwards with a small compute shader.")
            ("no-camera-selection", "Do not switch cameras on I frames and only use the first 3 given cameras the whole time.")
            ("background", "The RGB color of the background, as 3 ints in [0,255]", cxxopts::value<std::vector<int>>()->default_value("128,128,128"))
            ("use-camera-up", "Use the up vector as seen from the first input camera, instead of using real up.")
            ("use-camera-depth", "Use the near and far planes from the first input camera, instead of using near-infinite ones.")
            ("camera-fast-speed", "The default speed to move around the scene.", cxxopts::value<float>()->default_value("0.005"))
            ("camera-precision-speed", "The precision speed to move around the scene.", cxxopts::value<float>()->default_value("0.0005"))
            ("camera-fast-sensitivity", "The default sensitivity speed to look around the scene.", cxxopts::value<float>()->default_value("80"))
            ("camera-precision-sensitivity", "The precision sensitivity speed to look around the scene.", cxxopts::value<float>()->default_value("20"))
			("application-width", "The width of the application window.", cxxopts::value<int>()->default_value("1000"))
            ("application-height", "The height of the application window.", cxxopts::value<int>()->default_value("800"))
            ("nr-cameras", "The number of texture cameras. [1-3]", cxxopts::value<int>()->default_value("3"))
            ("static-view-camera", "Do not use a dynamic camera and select a static view camera instead.", cxxopts::value<int>()->default_value("-1"))
		    ;
        options.add_options("Precomputing Vertex Options")
			("edge-cutoff", "Decides when sobel-edges will be chosen as vertex edges. [0,255]", cxxopts::value<float>()->default_value("20"))
            ("edge-scaling", "Decides how much to expand the edges (thicker edges can yield better results). [0,]", cxxopts::value<int>()->default_value("0"))
            ("triangle-deletion-margin", "Cull triangles if the depth difference exceeds this margin (+ rule for triangle-straight-margin). [0,]", cxxopts::value<float>()->default_value("0.07"))
		    ("triangle-straight-margin", "Cull triangles if the pixel difference is within this margin (+ rule for triangle-deletion-margin). [0,]", cxxopts::value<int>()->default_value("10"))
            ("use-colored-edges", "Run edge detection on the colored texture file instead of the depth map for a more detailed, but more expensive mesh.")
            ("inner-position-epsilon", "Merge vertices if their distance is within this epsilon within a frame. [0,]", cxxopts::value<float>()->default_value("0"))//0.005
            ("outer-position-epsilon", "Merge vertices if their distance is within this epsilon across frames. [0,]", cxxopts::value<float>()->default_value("0"))//0.02
            ;
        options.add_options("Precomputing Optimizations")
			("max-pos-x", "Maximum x position to expect for a vertex.", cxxopts::value<int>()->default_value("2"))
            ("min-pos-x", "Minimum x position to expect for a vertex.", cxxopts::value<int>()->default_value("-2"))
		    ("x-partitions", "Divide the vertex field into this many partitions across the x dimension to optimize comparing vertices. [0,]", cxxopts::value<int>()->default_value("30"))
            ("max-pos-y", "Maximum y position to expect for a vertex.", cxxopts::value<int>()->default_value("1"))
            ("min-pos-y", "Minimum y position to expect for a vertex.", cxxopts::value<int>()->default_value("-1"))
		    ("y-partitions", "Divide the vertex field into this many partitions across the y dimension to optimize comparing vertices. [0,]", cxxopts::value<int>()->default_value("20"))
            ("max-pos-z", "Maximum z position to expect for a vertex.", cxxopts::value<int>()->default_value("5"))
            ("min-pos-z", "Minimum z position to expect for a vertex.", cxxopts::value<int>()->default_value("1"))
		    ("z-partitions", "Divide the vertex field into this many partitions across the z dimension to optimize comparing vertices. [0,]", cxxopts::value<int>()->default_value("30"))
            ;
        options.add_options("Precomputing & Rendering Options")
			("s,start-frame", "The frame to start rendering from. [0,]", cxxopts::value<int>()->default_value("0"))
            ("l,limit-frames", "The upper limit in the amount of frames to render, -1 to render all remaining frames. ", cxxopts::value<int>()->default_value("-1"))
            ("c,cameras", "Limit the cameras to render from. [0,]", cxxopts::value<std::vector<int>>())
		    ;
        options.add_options("Rendering Optimizations")
            ("no-vsync", "Disable V-Sync and the frame rate cap (same as display).")
			("parallelize-size", "The size of the thread pool and the amount of things we can parallelize. [1,]", cxxopts::value<int>()->default_value("8"))
            ("texture-lookahead", "The amount of textures to lookahead/buffer in parallel. [0,]", cxxopts::value<int>()->default_value("8"))
            ("texture-projection-bias", "The small bias on the depths when using a depth buffer.", cxxopts::value<float>()->default_value("0.005"))
            ("x-reach", "The x reach of the hole filling kernel. [0,]", cxxopts::value<int>()->default_value("5"))
            ("y-reach", "The y reach of the hole filling kernel. [0,]", cxxopts::value<int>()->default_value("5"))
            ("camera-grid-x-partitions", "The amount of x partitions in the camera selection grid. [1,]", cxxopts::value<int>()->default_value("100"))
            ("camera-grid-y-partitions", "The amount of y partitions in the camera selection grid. [1,]", cxxopts::value<int>()->default_value("40"))
            ("camera-grid-width", "The width of the camera selection grid. ]0,]", cxxopts::value<float>()->default_value("1"))
            ("camera-grid-height", "The width of the camera selection grid. ]0,]", cxxopts::value<float>()->default_value("1"))
            ("camera-grid-distance", "The distance of the camera selection grid. ]0,]", cxxopts::value<float>()->default_value("2"))
		    ;
        options.add_options("Generic Options")
			("only-precomputing", "Exit after precomputing and do not provide a live view of the light field.")
            ("single-frame", "Exit after rendering a single rendered frame.")
		    ;

    options.parse_positional({"json-file"});
    cxxopts::ParseResult result = options.parse(argc, argv);
    // print help if necessary
    if (argc < 2 || result.count("help"))
    {
        std::cout << options.help({ "Input Files" , "Output Files", "View Options", "Precomputing Vertex Options", "Precomputing Optimizations", "Precomputing & Rendering Options", "Rendering Optimizations", "Generic Options"}) << std::endl;
        exit(0);
    }

    json_file = result["json-file"].as<std::string>();
    input_binary = result["input-binary"].as<std::string>();

    output_binary = result["output-binary"].as<std::string>();
    output_edges = result["output-edge-images"].as<std::string>();
    output_color = result["output-color-images"].as<std::string>();
    output_depth = result["output-depth-images"].as<std::string>();
    output_render = result["output-render"].as<std::string>();
    output_times = result["output-times"].as<std::string>();

    noTexture = result["no-texture"].as<bool>();
    noDepthBuffer = result["no-depth-buffer"].as<bool>();
    noHoleFilling = result["no-hole-filling"].as<bool>();
    noCameraSelection = result["no-camera-selection"].as<bool>();
    textureBias = result["texture-projection-bias"].as<float>();
    staticViewCamera = result["static-view-camera"].as<int>();
    singleFrame = result["single-frame"].as<bool>();

    noVsync = result["no-vsync"].as<bool>();

    std::vector<int> b = result["background"].as<std::vector<int>>();
    if (b.size() > 0 && b.size() != 3) {
        std::cout << "Background needs to be followed by 3 ints, using --background 137,207,240" << std::endl;
        background = glm::vec3(137 / 255.0f, 207 / 255.0f, 240 / 255.0f);
    }
    else if (b.size() == 3) {
        background = glm::vec3(b[0] / 255.0f, b[1] / 255.0f, b[2] / 255.0f);
    }
    if (background.x < 0 || background.x > 1 || background.y < 0 || background.y > 1 || background.z < 0 || background.z > 1) {
        std::cout << "Background needs to be followed by 3 ints that lie within [0,255]" << std::endl;
        exit(-1);
    }
    cameraUp = result["use-camera-up"].as<bool>();
    cameraDepth = result["use-camera-depth"].as<bool>();
    cameraFastSpeed = result["camera-fast-speed"].as<float>();
    cameraPrecisionSpeed = result["camera-precision-speed"].as<float>();
    cameraFastSensitivity = result["camera-fast-sensitivity"].as<float>();
    cameraPrecisionSensitivity = result["camera-precision-sensitivity"].as<float>();

    application_width = result["application-width"].as<int>();
    if (application_width <= 0)
    {
        std::cout << "Application width cannot be negative or zero, converting to default value of 1000." << std::endl;
        application_width = 1000;
    }
    application_height = result["application-height"].as<int>();
    if (application_height <= 0)
    {
        std::cout << "Application height cannot be negative or zero, converting to default value of 800." << std::endl;
        application_height = 800;
    }

    edge_cutoff = result["edge-cutoff"].as<float>();
    if (edge_cutoff < 0)
    {
        std::cout << "Edge-cutoff cannot be negative, clipping to zero (everything will be an edge, please increase the cutoff if this is unwanted behaviour)." << std::endl;
        edge_cutoff = 0;
    }
    edge_scaling = result["edge-scaling"].as<int>();
    if (edge_scaling < 0)
    {
        std::cout << "Edge-scaling cannot be negative, clipping to zero (no scaling behaviour)." << std::endl;
        edge_scaling = 0;
    }
    triangle_deletion_margin = result["triangle-deletion-margin"].as<float>();
    if (triangle_deletion_margin < 0)
    {
        std::cout << "Triangle deletion margin cannot be negative, clipping to zero." << std::endl;
        triangle_deletion_margin = 0;
    }
    triangle_straight_margin = result["triangle-straight-margin"].as<int>();
    if (triangle_straight_margin < 0)
    {
        std::cout << "Triangle straight margin cannot be negative, clipping to zero." << std::endl;
        triangle_straight_margin = 0;
    }
    use_colored_edges = result["use-colored-edges"].as<bool>();
    inner_position_epsilon = result["inner-position-epsilon"].as<float>();
    if (inner_position_epsilon < 0)
    {
        std::cout << "Inner position epsilon cannot be negative, clipping to zero." << std::endl;
        inner_position_epsilon = 0;
    }
    outer_position_epsilon = result["outer-position-epsilon"].as<float>();
    if (outer_position_epsilon < 0)
    {
        std::cout << "Outer position epsilon cannot be negative, clipping to zero." << std::endl;
        outer_position_epsilon = 0;
    }

    max_pos_x = result["max-pos-x"].as<int>();
    min_pos_x = result["min-pos-x"].as<int>();
    x_partitions = result["x-partitions"].as<int>();
    if (x_partitions < 0)
    {
        std::cout << "The number of x partitions cannot be negative, clipping to zero." << std::endl;
        x_partitions = 0;
    }
    max_pos_y = result["max-pos-y"].as<int>();
    min_pos_y = result["min-pos-y"].as<int>();
    y_partitions = result["y-partitions"].as<int>();
    if (y_partitions < 0)
    {
        std::cout << "The number of y partitions cannot be negative, clipping to zero." << std::endl;
        y_partitions = 0;
    }
    max_pos_z = result["max-pos-z"].as<int>();
    min_pos_z = result["min-pos-z"].as<int>();
    z_partitions = result["z-partitions"].as<int>();
    if (z_partitions < 0)
    {
        std::cout << "The number of z partitions cannot be negative, clipping to zero." << std::endl;
        z_partitions = 0;
    }

    startFrame = result["start-frame"].as<int>();
    if (startFrame < 0)
    {
        std::cout << "The start frame must be positive, starting from zero." << std::endl;
        startFrame = 0;
    }
    limitFrames = result["limit-frames"].as<int>();
    if (limitFrames <= 0 && limitFrames != -1)
    {
        std::cout << "The amount of frames cannot be negative, only -1 allowed => defaulting" << std::endl;
        limitFrames = -1;
    }
    if (result["cameras"].count())
    {
        cameras = result["cameras"].as<std::vector<int>>();
    }
    else
    {
        cameras = std::vector<int>();
    }
        

    thread_pool_size = result["parallelize-size"].as<int>();
    if (thread_pool_size < 1)
    {
        std::cout << "We should be allowed to run at least one thread in the pool, defaulting to 1" << std::endl;
        thread_pool_size = 1;
    }
    texture_lookahead = result["texture-lookahead"].as<int>();
    if (texture_lookahead < 0)
    {
        std::cout << "Texture lookahead should be positive, defaulting to 5." << std::endl;
        texture_lookahead = 5;
    }
    x_reach = result["x-reach"].as<int>();
    if (x_reach < 0)
    {
        std::cout << "The x-reach must be positive, starting from zero." << std::endl;
        x_reach = 0;
    }
    y_reach = result["y-reach"].as<int>();
    if (y_reach < 0)
    {
        std::cout << "The y-reach must be positive, starting from zero." << std::endl;
        y_reach = 0;
    }

    nrCameras = result["nr-cameras"].as<int>();
    if (nrCameras < 1 || nrCameras > 3)
    {
        std::cout << "nrCameras must be between 1 and 3, defaulting to 3" << std::endl;
        nrCameras = 3;
    }

    camera_grid_x_partitions = result["camera-grid-x-partitions"].as<int>();
    if (camera_grid_x_partitions <= 0)
    {
        std::cout << "Camera grid x partitions must be greater than 0" << std::endl;
        camera_grid_x_partitions = 1;
    }
    camera_grid_y_partitions = result["camera-grid-y-partitions"].as<int>();
    if (camera_grid_y_partitions <= 0)
    {
        std::cout << "Camera grid y partitions must be greater than 0" << std::endl;
        camera_grid_y_partitions = 1;
    }

    camera_grid_width = result["camera-grid-width"].as<float>();
    if (camera_grid_width <= 0)
    {
        std::cout << "Camera grid width must be greater than 0" << std::endl;
        camera_grid_width = 1;
    }
    camera_grid_height = result["camera-grid-height"].as<float>();
    if (camera_grid_height <= 0)
    {
        std::cout << "Camera grid height must be greater than 0" << std::endl;
        camera_grid_height = 1;
    }
    camera_grid_distance = result["camera-grid-distance"].as<float>();
    if (camera_grid_distance <= 0)
    {
        std::cout << "Camera grid distance must be greater than 0" << std::endl;
        camera_grid_distance = 1;
    }

    only_precomputing = result["only-precomputing"].as<bool>();
}