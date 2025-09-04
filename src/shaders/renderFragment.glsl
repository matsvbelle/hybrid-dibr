#version 460 core

// Outputs colors in RGBA
out vec4 FragColor;

struct sampleOutput
{
	vec2 texCoords;
    float depth;
};

struct cameraInput
{
	mat4 model;
	vec2 focal;
	vec2 pp;
	vec2 res;
	float z_far;
	float z_near;
};

// Inputs the color from the Vertex Shader
in fs_in
{
    vec3 color;
    sampleOutput samples[3];
}frag;

uniform sampler2D luminanceSampler1;
uniform sampler2D chromaSampler1;
uniform sampler2D depthSampler1;

uniform sampler2D luminanceSampler2;
uniform sampler2D chromaSampler2;
uniform sampler2D depthSampler2;

uniform sampler2D luminanceSampler3;
uniform sampler2D chromaSampler3;
uniform sampler2D depthSampler3;

uniform cameraInput cameras[3];

uniform float bias;
uniform int nrTextures;

uniform bool sampleColors;
uniform bool sampleDepth;

vec3 sampleRGB(int i)
{
    unsigned char luminance;
    if (i == 0) luminance = unsigned char(texture(luminanceSampler1, frag.samples[0].texCoords).x * 255);
    if (i == 1) luminance = unsigned char(texture(luminanceSampler2, frag.samples[1].texCoords).x * 255);
    if (i == 2) luminance = unsigned char(texture(luminanceSampler3, frag.samples[2].texCoords).x * 255);

    unsigned char cb;
    if (i == 0) cb = unsigned char(texture(chromaSampler1, frag.samples[0].texCoords).x * 255);
    if (i == 1) cb = unsigned char(texture(chromaSampler2, frag.samples[1].texCoords).x * 255);
    if (i == 2) cb = unsigned char(texture(chromaSampler3, frag.samples[2].texCoords).x * 255);

    unsigned char cr;
    if (i == 0) cr = unsigned char(texture(chromaSampler1, frag.samples[0].texCoords).y * 255);
    if (i == 1) cr = unsigned char(texture(chromaSampler2, frag.samples[1].texCoords).y * 255);
    if (i == 2) cr = unsigned char(texture(chromaSampler3, frag.samples[2].texCoords).y * 255);

    unsigned char r = unsigned char(float(luminance) + 1.402*(float(cr)-128));
    unsigned char g = unsigned char(float(luminance) - 0.344*(float(cb)-128) - 0.714*(float(cr)-128));
    unsigned char b = unsigned char(float(luminance) + 1.772*(float(cb)-128));

    return vec3(r/255.0f, g/255.0f, b/255.0f);
}

float sampleClosestDepth(int i)
{
    if (i == 0) return texture(depthSampler1, frag.samples[0].texCoords).x;
    if (i == 1) return texture(depthSampler2, frag.samples[1].texCoords).x;
    if (i == 2) return texture(depthSampler3, frag.samples[2].texCoords).x;
}

void main()
{
    
    int sampledAmount = 0;
    if (sampleColors)
    {   
        vec3 color = vec3(0,0,0);
        for (int i = 0; i < nrTextures; ++i)
        {
            if (frag.samples[i].texCoords.x >= 0 && frag.samples[i].texCoords.x <= 1 && frag.samples[i].texCoords.y >= 0 && frag.samples[i].texCoords.y <= 1)
            {
                bool mustSample = true;
                if (sampleDepth)
                {
                    float closestDepth = sampleClosestDepth(i);
                    float currentDepth = (frag.samples[i].depth-cameras[i].z_near) / (cameras[i].z_far - cameras[i].z_near);

                    if (currentDepth > closestDepth + bias)
                    {
                        mustSample = false;
                    }
                }
                
                if (mustSample)
                {
                    color += sampleRGB(i);
                    sampledAmount += 1;
                }
            }
        }
        if (sampledAmount > 0)
        {
            FragColor = vec4(color/sampledAmount, 1.0f);
        }
    }
    
    if (sampledAmount == 0)
    {
        FragColor = vec4(frag.color, 1.0f);
    }
}