#include "Reader.h"

struct ComputeVertex
{
	glm::vec3 position;
	glm::vec3 color;
	float depth;
    int row;
    int col;
};

struct OldVertexPosition
{
    glm::vec3 position;
    int index;
};

Reader::Reader(std::string json_file, Options s_options)
{
    cameras = InputCamera::readCameraInputJson(json_file);
    options = s_options;
}

Reader::Reader(std::vector<InputCamera> input_cameras, Options s_options)
{
    cameras = input_cameras;
    options = s_options;
}

void Reader::parse()
{
    _allVertices = std::vector<std::vector<Vertex>>();
    _allIndices = std::vector<std::vector<GLuint>>();

    int nrFrames = cameras[0].intrinsics.frames - options.startFrame;
    if (options.limitFrames != -1 && options.limitFrames < nrFrames)
    {
        nrFrames = options.limitFrames;
    }
    std::vector<int> camera_vector = options.cameras;
    if (camera_vector.empty())
    {
        for (int i = 0; i < cameras.size(); ++i)
        {
            camera_vector.push_back(i);
        }
    }

    int xmin = 999999;
    int xmax = -999999;
    int ymin = 999999;
    int ymax = -999999;
    int zmin = 999999;
    int zmax = -999999;

    double startTime = glfwGetTime();
    int completeSize = 0;

    for (size_t i = options.startFrame+1; i < options.startFrame+1+nrFrames; ++i)
    {
        std::vector<Vertex> verticesVector;
        std::vector<GLuint> indicesVector;
        int skipped_count = 0;

        std::vector<std::vector<OldVertexPosition>> oldVertexPositions = std::vector<std::vector<OldVertexPosition>>(options.x_partitions*options.y_partitions*options.z_partitions);

        for (int cameraNr : camera_vector)
        {
            std::cout << "calculating frame " << i << " for camera " << cameraNr << "..." << std::endl;
            InputCamera camera = cameras[cameraNr];

            int width = camera.intrinsics.res[0];
            int height = camera.intrinsics.res[1];

            int nrVertices = width * height;
            ComputeVertex* vertices = new ComputeVertex[nrVertices];
            int* vertexIndices = new int[nrVertices];

            // Get position and color
            std::cout << "    " << "Getting color and depth images..." << std::endl;
            unsigned char* colored_pixels = get_colored_frame(camera.jsonDirectory + camera.nameColor, i);
            float* depth_pixels = get_depth_frame(camera.jsonDirectory + camera.nameDepth, i);
            if (options.output_color != "")
            {
                writeImage(options.output_color + std::to_string(i) + "v" + std::to_string(cameraNr) + "color.png", colored_pixels, width, height, 3);
            }
            if (options.output_depth != "")
            {   
                unsigned char* depth_temp = new unsigned char[nrVertices * 3];
                for (int i = 0; i < nrVertices; ++i)
                {
                    depth_temp[i*3] = int(depth_pixels[i] * 255.0);
                    depth_temp[i*3+1] = int(depth_pixels[i] * 255.0);
                    depth_temp[i*3+2] = int(depth_pixels[i] * 255.0);
                }
                writeImage(options.output_depth + std::to_string(i) + "v" + std::to_string(cameraNr) + "depth.png", depth_temp, width, height, 3);
                delete depth_temp;
            }
            std::cout <<  "    " << "Color and depth images loaded." << std::endl;

            // get the position and color for each pixel
            std::cout <<  "    " << "Starting Compute Shader..." << std::endl;
            int* edges = new int[nrVertices];

            SSBO vertex_ssbo;
            vertex_ssbo.LinkData(vertices, nrVertices * sizeof(ComputeVertex), 1);
            SSBO color_ssbo;
            color_ssbo.LinkData(colored_pixels, 3 * nrVertices * sizeof(unsigned char), 2);
            SSBO depth_ssbo;
            depth_ssbo.LinkData(depth_pixels, nrVertices * sizeof(float), 3);
            SSBO edge_ssbo;
            edge_ssbo.LinkData(edges, nrVertices * sizeof(int), 4);
            SSBO edge_ssbo_receive;
            edge_ssbo_receive.LinkData(edges, nrVertices * sizeof(int), 5);
            
            delete depth_pixels;
            delete colored_pixels;

            ComputeShader pointCloudCompute("pointCloudCompute.glsl");
            pointCloudCompute.Activate();

            glUniformMatrix4fv(glGetUniformLocation(pointCloudCompute.ID, "model"), 1, GL_FALSE, glm::value_ptr(camera.model));
            glUniform1f(glGetUniformLocation(pointCloudCompute.ID, "z_far"), camera.z_far);
            glUniform1f(glGetUniformLocation(pointCloudCompute.ID, "z_near"), camera.z_near);
            glUniform2fv(glGetUniformLocation(pointCloudCompute.ID, "pp"), 1, glm::value_ptr(camera.pp));
            glUniform2fv(glGetUniformLocation(pointCloudCompute.ID, "focal"), 1, glm::value_ptr(camera.intrinsics.focal));
            glUniform1f(glGetUniformLocation(pointCloudCompute.ID, "edge_cutoff"), options.edge_cutoff);
            glUniform1i(glGetUniformLocation(pointCloudCompute.ID, "use_colored_edges"), options.use_colored_edges);

            pointCloudCompute.Dispatch(width, height, 1);
            pointCloudCompute.Delete();
            std::cout <<  "    " << "Compute Shader ended." << std::endl;

            std::cout <<  "    " << "Obtaining Vertex Data..." << std::endl;
            color_ssbo.Delete();
            depth_ssbo.Delete();

            vertex_ssbo.GetData(vertices, nrVertices * sizeof(ComputeVertex));
            vertex_ssbo.Delete();
            std::cout <<  "    " << "Vertex Data Obtained..." << std::endl;

            std::cout <<  "    " << "Computing Edges..." << std::endl;
            if (options.edge_scaling > 0)
            {
                ComputeShader edgeCompute("edgeScaleCompute.glsl");
                edgeCompute.Activate();

                glUniform1i(glGetUniformLocation(edgeCompute.ID, "edge_scaling"), options.edge_scaling);

                edgeCompute.Dispatch(width, height, 1);
                edgeCompute.Delete();
            }
            edge_ssbo.Delete();

            edge_ssbo_receive.GetData(edges, nrVertices * sizeof(int));
            edge_ssbo_receive.Delete();

            if (options.output_edges != "")
            {
                unsigned char* edge_pixels = new unsigned char[3*nrVertices];
                for (int row = 0; row < height; ++row) 
                {
                    for (int col = 0; col < width; ++col) 
                    {   
                        int pixel = row*width + col;
                        if (edges[pixel] == -1)
                        {
                            edge_pixels[3*pixel] = (unsigned char)255;
                            edge_pixels[3*pixel+1] = (unsigned char)255;
                            edge_pixels[3*pixel+2] = (unsigned char)255;
                        }
                        else
                        {
                            edge_pixels[3*pixel] = (unsigned char)0;
                            edge_pixels[3*pixel+1] = (unsigned char)0;
                            edge_pixels[3*pixel+2] = (unsigned char)0;
                        }
                    }
                }
                writeImage(options.output_edges + std::to_string(i) + "v" + std::to_string(cameraNr) + "edges.png", edge_pixels, width, height, 3);
                delete edge_pixels;
            }
            std::cout <<  "    " << "Edges obtained..." << std::endl;

            std::cout <<  "    " << "Parsing Vertices..." << std::endl;
            std::vector<double> coords;
            int previous_vector_size = int(verticesVector.size());
            for (int i = 0; i < nrVertices; ++i)
            {
                if (edges[i] == -1)
                {   
                    vertexIndices[i] = -1;

                    int partition_x = int((vertices[i].position.x - options.min_pos_x) / ((options.max_pos_x - options.min_pos_x) / float(options.x_partitions)));
                    if (partition_x >= options.x_partitions) partition_x = options.x_partitions-1;
                    if (partition_x < 0) partition_x = 0;
                    int partition_y = int((vertices[i].position.y - options.min_pos_y) / ((options.max_pos_y - options.min_pos_y) / float(options.y_partitions)));
                    if (partition_y >= options.y_partitions) partition_y = options.y_partitions-1;
                    if (partition_y < 0) partition_y = 0;
                    int partition_z = int((vertices[i].position.z - options.min_pos_z) / ((options.max_pos_z - options.min_pos_z) / float(options.z_partitions)));
                    if (partition_z >= options.z_partitions) partition_z = options.z_partitions-1;
                    if (partition_z < 0) partition_z = 0;

                    if (vertices[i].position.x < xmin) xmin = int(vertices[i].position.x);
                    if (vertices[i].position.x > xmax) xmax = int(vertices[i].position.x);
                    if (vertices[i].position.y < ymin) ymin = int(vertices[i].position.y);
                    if (vertices[i].position.y > ymax) ymax = int(vertices[i].position.y);
                    if (vertices[i].position.z < zmin) zmin = int(vertices[i].position.z);
                    if (vertices[i].position.y > zmax) zmax = int(vertices[i].position.z);

                    int oldVertexPartition = partition_x*options.y_partitions*options.z_partitions + partition_y*options.z_partitions + partition_z;

                    if(options.inner_position_epsilon != 0 || options.inner_position_epsilon != 0)
                    {
                        for (OldVertexPosition old_pos : oldVertexPositions[oldVertexPartition])
                        {
                            float position_epsilon = options.inner_position_epsilon;
                            if (old_pos.index < previous_vector_size)
                            {
                                position_epsilon = options.outer_position_epsilon;
                            }
                            if (position_epsilon == 0)
                            {
                                continue;
                            }
                            if (sqrt(pow(vertices[i].position.x - old_pos.position.x, 2) + pow(vertices[i].position.y - old_pos.position.y, 2) + pow(vertices[i].position.z - old_pos.position.z, 2)) < position_epsilon)
                            {
                                vertexIndices[i] = old_pos.index;
                                break;
                            }
                        }
                    }

                    if (vertexIndices[i] < previous_vector_size)
                    {
                        coords.push_back((double)vertices[i].row);
                        coords.push_back((double)vertices[i].col);
                    }
                    
                    if (vertexIndices[i] == -1)
                    {
                        vertexIndices[i] = int(verticesVector.size());
                        verticesVector.push_back(Vertex{vertices[i].position, vertices[i].color});
                        oldVertexPositions[oldVertexPartition].push_back(OldVertexPosition{vertices[i].position, vertexIndices[i]});
                    } 
                    else
                    {
                        ++skipped_count;
                    }
                }
            }
            delete edges;
            std::cout <<  "    " << "All vertices parsed." << std::endl;

            std::cout <<  "    " << "Triangulating..." << std::endl;
            delaunator::Delaunator triangulation(coords);
            std::cout <<  "    " << "Triangulation completed." << std::endl;

            std::cout <<  "    " << "Parsing Indices..." << std::endl;
            for(size_t i = 0; i < triangulation.triangles.size(); i+=3)
            {
                glm::vec2 position_a = glm::vec2(triangulation.coords[2 * triangulation.triangles[i]], triangulation.coords[2 * triangulation.triangles[i] + 1]);
                glm::vec2 position_b = glm::vec2(triangulation.coords[2 * triangulation.triangles[i+1]], triangulation.coords[2 * triangulation.triangles[i+1] + 1]);
                glm::vec2 position_c = glm::vec2(triangulation.coords[2 * triangulation.triangles[i+2]], triangulation.coords[2 * triangulation.triangles[i+2] + 1]);

                int vertex_position_a = int(position_a.x*width + position_a.y);
                int vertex_position_b = int(position_b.x*width + position_b.y);
                int vertex_position_c = int(position_c.x*width + position_c.y);

                float largest_depth_diff = std::max(abs(vertices[vertex_position_a].depth-vertices[vertex_position_b].depth), std::max(
                                                    abs(vertices[vertex_position_a].depth-vertices[vertex_position_c].depth), 
                                                    abs(vertices[vertex_position_b].depth-vertices[vertex_position_c].depth)));

                if ((largest_depth_diff > options.triangle_deletion_margin) &&
                    abs(int(position_a.x - position_b.x)) < options.triangle_straight_margin && abs(int(position_a.x - position_c.x)) < options.triangle_straight_margin && abs(int(position_b.x - position_c.x)) < options.triangle_straight_margin &&
                    abs(int(position_a.y - position_b.y)) < options.triangle_straight_margin && abs(int(position_a.y - position_c.y)) < options.triangle_straight_margin && abs(int(position_b.y - position_c.y)) < options.triangle_straight_margin)
                {
                    continue;
                }
                if (!(vertex_position_a < previous_vector_size && vertex_position_b < previous_vector_size && vertex_position_c < previous_vector_size))
                {
                    indicesVector.push_back(vertexIndices[vertex_position_a]);
                    indicesVector.push_back(vertexIndices[vertex_position_b]);
                    indicesVector.push_back(vertexIndices[vertex_position_c]);
                }
            }
            std::cout <<  "    " << "All indices parsed." << std::endl;
            delete vertices;
            delete vertexIndices;
        }
        completeSize += verticesVector.size();
        std::cout << "Obtained a frame with " << verticesVector.size() << " vertices!" << std::endl;
        std::cout << "Skipped " << skipped_count << " vertices!" << std::endl;
        _allVertices.push_back(verticesVector);
        _allIndices.push_back(indicesVector);
    }

    std::cout << "Done with precomputing the scene mesh, took " << glfwGetTime() - startTime << " seconds." << std::endl;
    std::cout << "Full mesh is " << completeSize << " vertices, average of " << completeSize / _allVertices.size() << " vertices per frame." << std::endl;
    std::cout << "For faster computation next time (x-min=" << xmin << ", x-max=" << xmax << ", y-min=" << ymin << ", y-max=" << ymax << ", z-min=" << zmin << " & z-max=" << zmax << ") can be chosen." << std::endl;
}

void Reader::exportBinary(std::string binary_file)
{
    std::ofstream output(binary_file, std::ios::binary);

    uint64_t amountOfFrames = _allVertices.size();
    output.write((const char*)&amountOfFrames, sizeof(uint64_t));

    for (size_t frameNumber = 0; frameNumber < amountOfFrames; ++frameNumber)
    {
        uint64_t amountOfVertices = _allVertices[frameNumber].size();
        uint64_t amountOfIndices = _allIndices[frameNumber].size();

        output.write((const char*)&amountOfVertices, sizeof(uint64_t));
        output.write((const char*)&amountOfIndices, sizeof(uint64_t));

        for (int vertexNumber = 0; vertexNumber < amountOfVertices; ++vertexNumber)
        {
            output.write((const char*)(&_allVertices[frameNumber][vertexNumber]), sizeof(Vertex));
        }
        for (int indexNumber = 0; indexNumber < amountOfIndices; ++indexNumber)
        {
            output.write((const char*)(&_allIndices[frameNumber][indexNumber]), sizeof(GLuint));
        }
    }
	output.close();
}

void Reader::readBinary(std::string binary_file)
{
    std::ifstream input(binary_file, std::ios::binary);

    uint64_t amountOfFrames;
    input.read((char*)(&amountOfFrames), sizeof(uint64_t));

    _allVertices = std::vector<std::vector<Vertex>>(amountOfFrames);
    _allIndices = std::vector<std::vector<GLuint>>(amountOfFrames);

    for (size_t frameNumber = 0; frameNumber < amountOfFrames; ++frameNumber)
    {
        uint64_t amountOfVertices, amountOfIndices;
        input.read((char*)(&amountOfVertices), sizeof(uint64_t));
        input.read((char*)(&amountOfIndices), sizeof(uint64_t));

        std::vector<Vertex> vertices = std::vector<Vertex>(amountOfVertices);
        for (int vertexNumber = 0; vertexNumber < amountOfVertices; ++vertexNumber)
        {
            input.read((char*)(&vertices[vertexNumber]), sizeof(Vertex));
        }
        _allVertices[frameNumber] = vertices;

        std::vector<GLuint> indices = std::vector<GLuint>(amountOfIndices);
        for (int indexNumber = 0; indexNumber < amountOfIndices; ++indexNumber)
        {
            input.read((char*)(&indices[indexNumber]), sizeof(GLuint));
        }
        _allIndices[frameNumber] = indices;
    }
}

DynamicMesh Reader::createDynamicMesh()
{
    return DynamicMesh(_allVertices, _allIndices);
}

