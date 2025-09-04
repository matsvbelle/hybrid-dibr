#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct ComputeVertex
{
	float position_x;
    float position_y;
    float position_z;
	float color_x;
    float color_y;
    float color_z;
	float depth;
    int row;
    int col;
};

layout(std430, binding = 1) writeonly buffer bufferDataVertices {
    ComputeVertex[] vertices;
};

layout(std430, binding = 2) readonly buffer bufferDataColorPixels {
    unsigned char[] colored_pixels;
};

layout(std430, binding = 3) readonly buffer bufferDataDepthPixels {
    float[] depth_pixels;
};

layout(std430, binding = 4) writeonly buffer bufferDataEdgePixels {
    int[] edge_pixels;
};

layout(std430, binding = 5) writeonly buffer bufferDataEdgeReceivePixels {
    int[] edge_receive_pixels;
};


// Import the uniform camera parameters
uniform mat4 model;

uniform float z_far;
uniform float z_near;

uniform vec2 pp;
uniform vec2 focal;

uniform float edge_cutoff;
uniform bool use_colored_edges;

float sobel_edge(int row, int col)
{
    int depth_slices = 20;
    if (row == 0 || col == 0 || row == int(gl_NumWorkGroups.y)-1 || col == int(gl_NumWorkGroups.x)-1)
    {
        return 255;
    }
    float kx[3][3] = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};
	float ky[3][3] = {{-1, -2, -1},{0, 0, 0},{1, 2, 1}};

    float gx = 0;
    float gy = 0;
    for (int row_conv = -1; row_conv <= 1; ++row_conv)
    {
        for (int col_conv = -1; col_conv <= 1; ++col_conv)
        {
            int row_pos = row - row_conv;
            int col_pos = col - col_conv;
            int pixel = row_pos*int(gl_NumWorkGroups.x) + col_pos;
            
            if (use_colored_edges)
            {
                gx += kx[row_conv+1][col_conv+1] * (colored_pixels[3*pixel] + colored_pixels[3*pixel+1] + colored_pixels[3*pixel+2])/3.0;
                gy += ky[row_conv+1][col_conv+1] * (colored_pixels[3*pixel] + colored_pixels[3*pixel+1] + colored_pixels[3*pixel+2])/3.0;
            }
            else
            {
                gx += kx[row_conv+1][col_conv+1] * (int(depth_pixels[pixel]*1024)/depth_slices)*depth_slices;
                gy += ky[row_conv+1][col_conv+1] * (int(depth_pixels[pixel]*1024)/depth_slices)*depth_slices;
            }
        }
    }
    return sqrt(gx*gx + gy*gy) / 4.0;
}

void main()
{
    float width_f = float(gl_NumWorkGroups.x);
    float height_f = float(gl_NumWorkGroups.y);
    int row = int(gl_GlobalInvocationID.y);
    int col = int(gl_GlobalInvocationID.x);

    // 2 floats (u and v) per pixel
    float u = col / width_f;
    float v = row / height_f;
    int pixel = row*int(gl_NumWorkGroups.x) + col;

    // get the depth value in [0,1]
    float depth = depth_pixels[pixel];
    // convert to depth in [near, far] (in meters) by scaling with near and far planes
    depth = 1.0 / (1.0f / z_far + depth * ( 1.0f / z_near - 1.0f / z_far));

    // perspective unprojection
    float x = (col + 0.5 - pp.x) / focal.x * depth;
    float y = (row + 0.5 - pp.y) / focal.y * depth;

    vec4 localPosition = vec4(x, y, depth, 1.0f);
    vec4 globalPosition = model * localPosition;
    vec3 position = vec3(-globalPosition.x, -globalPosition.y, globalPosition.z);

    vec3 color = vec3(colored_pixels[pixel*3]/255.0f, colored_pixels[pixel*3+1]/255.0f, colored_pixels[pixel*3+2]/255.0f);

    float edge = sobel_edge(row, col);

    vertices[pixel].position_x = position.x;
    vertices[pixel].position_y = position.y;
    vertices[pixel].position_z = position.z;
    vertices[pixel].color_x = color.x;
    vertices[pixel].color_y = color.y;
    vertices[pixel].color_z = color.z;
    vertices[pixel].depth = depth;
    vertices[pixel].row = row;
    vertices[pixel].col = col;

    if (edge >= edge_cutoff)
    {
        edge_pixels[pixel] = -1;
        edge_receive_pixels[pixel] = -1;
    } 
    else
    {
        edge_pixels[pixel] = 0;
        edge_receive_pixels[pixel] = 0;
    }
    
}