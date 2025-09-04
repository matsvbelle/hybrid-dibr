#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

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

out fs_in
{
    vec3 color;
	sampleOutput samples[3];
}vertex;

uniform mat4 camMatrix;

uniform cameraInput cameras[3];

uniform int nrTextures;

uniform bool sampleColors;
uniform bool staticView;

uniform cameraInput staticViewCamera;

void main()
{
	vertex.color = aColor;
	if (staticView)
	{
		vec4 cameraPosition = inverse(staticViewCamera.model) * vec4(-aPos.x, -aPos.y, aPos.z, 1.0);
		cameraPosition /= cameraPosition.w;

		float img_x = staticViewCamera.focal.x * (cameraPosition.x / cameraPosition.z) + staticViewCamera.pp.x - 0.5;
		float img_y = staticViewCamera.focal.y * (cameraPosition.y / cameraPosition.z) + staticViewCamera.pp.y - 0.5;

		vec2 uv = vec2(img_x/staticViewCamera.res.x, img_y/staticViewCamera.res.y); 
		float depth = (cameraPosition.z - staticViewCamera.z_near) / (staticViewCamera.z_far - staticViewCamera.z_near);

		gl_Position = vec4(uv.x * 2.0 - 1.0, -uv.y * 2.0 + 1.0, depth, 1.0);
	}
	else
	{
		gl_Position = camMatrix * vec4(aPos, 1.0);
	}
	
	if (sampleColors)
	{
		for (int i = 0; i <= nrTextures; ++i)
		{
			vec4 cameraPosition = inverse(cameras[i].model) * vec4(-aPos.x, -aPos.y, aPos.z, 1.0);
			cameraPosition /= cameraPosition.w;

			float img_x = cameras[i].focal.x * (cameraPosition.x / cameraPosition.z) + cameras[i].pp.x - 0.5;
			float img_y = cameras[i].focal.y * (cameraPosition.y / cameraPosition.z) + cameras[i].pp.y - 0.5;

			vertex.samples[i].texCoords = vec2(img_x/cameras[i].res.x, img_y/cameras[i].res.y); 
			vertex.samples[i].depth = cameraPosition.z;
		}
	}
}