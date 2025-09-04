#version 460 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;

uniform bool fillHoles;
uniform vec2 res;
uniform int x_reach;
uniform int y_reach;
uniform vec3 background;

void main()
{
    vec3 color = texture(screenTexture, texCoords).xyz;
    
    bool sampledNewColor = false;
    if (fillHoles && color == background)
    {
        float offset_x = 1.0f / res.x;  
        float offset_y = 1.0f / res.y;  
        float closestOffset = x_reach+1*offset_x + y_reach+1*offset_y;
        for (float x = -x_reach*offset_x; x <= x_reach*offset_x; x+=offset_x)
        {
            for (float y = -y_reach*offset_y; y <= y_reach*offset_y; y+=offset_y)
            {
                vec2 sampleCoords = texCoords.st + vec2(x, y);
                if (sampleCoords.x >= 0 && sampleCoords.x <= 1 && sampleCoords.y >= 0 && sampleCoords.y <= 1)
                {
                    vec3 newSampledColor = texture(screenTexture, sampleCoords).xyz;
                    if (newSampledColor != background)
                    {
                        float newOffset = abs(x) + abs(y);
                        if (newOffset < closestOffset)
                        {
                            FragColor = vec4(newSampledColor, 1.0f);
                            closestOffset = newOffset;
                            sampledNewColor = true;
                        }
                    }
                }
            }
        }
    }

    if (!sampledNewColor)
    {
        FragColor = vec4(color, 1.0f);
    }
}