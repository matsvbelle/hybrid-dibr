#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 4) readonly buffer bufferDataEdgePixels {
    int[] edge_pixels;
};

layout(std430, binding = 5) writeonly buffer bufferDataEdgeReceivePixels {
    int[] edge_receive_pixels;
};

uniform int edge_scaling;

void main()
{
    float width_f = float(gl_NumWorkGroups.x);
    float height_f = float(gl_NumWorkGroups.y);
    int row = int(gl_GlobalInvocationID.y);
    int col = int(gl_GlobalInvocationID.x);

    int pixel = row*int(gl_NumWorkGroups.x) + col;

    if (edge_pixels[pixel] == -1)
    {
        for (int row_diff = edge_scaling * -1; row_diff <= edge_scaling; ++row_diff)
        {
            for (int col_diff = edge_scaling * -1; col_diff <= edge_scaling; ++col_diff)
            {
                int edge_pixel = (row+row_diff)*int(gl_NumWorkGroups.x) + (col+col_diff);
                edge_receive_pixels[edge_pixel] = -1;
            }
        }
    } 
}