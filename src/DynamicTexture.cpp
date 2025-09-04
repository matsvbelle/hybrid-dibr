#include "DynamicTexture.h"

#include <GLFW/glfw3.h>

DynamicTexture::DynamicTexture()
{}

DynamicTexture::DynamicTexture(Shader& shader, Options options, dp::thread_pool<>* pool, InputCamera textureCamera, TexUniform luminanceUniform, TexUniform chrominanceUniform)
{
	DynamicTexture::options = options;
	DynamicTexture::pool = pool;
	DynamicTexture::textureCamera = textureCamera;

	ThreadState thread_state;
	thread_state.nrFrames = textureCamera.intrinsics.frames - options.startFrame;
    if (options.limitFrames != -1 && options.limitFrames < thread_state.nrFrames)
    {
        thread_state.nrFrames = options.limitFrames;
    }
	thread_state.currentFrame = options.startFrame+1;
	DynamicTexture::static_texture = thread_state.nrFrames == 1;

	DynamicTexture::luminanceTexture = Texture(TextureData{GL_TEXTURE_2D, luminanceUniform.slot, GL_RED, GL_UNSIGNED_BYTE, GL_RED}, textureCamera.intrinsics.res[0], textureCamera.intrinsics.res[1]);
	luminanceTexture.texUnit(shader, luminanceUniform.uniform, luminanceUniform.unit);

	DynamicTexture::chrominanceTexture = Texture(TextureData{GL_TEXTURE_2D, chrominanceUniform.slot, GL_RG, GL_UNSIGNED_BYTE, GL_RG}, textureCamera.intrinsics.res[0]/2, textureCamera.intrinsics.res[1]/2);
	chrominanceTexture.texUnit(shader, chrominanceUniform.uniform, chrominanceUniform.unit);

	video_reader_open(&thread_state.vr_state, (textureCamera.jsonDirectory + textureCamera.nameColor).c_str());
	video_reader_seek_frame(&thread_state.vr_state, thread_state.currentFrame);

	
	for (int i = 0; i < thread_state.nrFrames-1; ++i)
	{
		if (thread_state.vr_state.av_frame->pict_type == AV_PICTURE_TYPE_I)
		{
			thread_state.i_frames.push_back(thread_state.currentFrame);
		}
		++thread_state.currentFrame;
		video_reader_next_frame(&thread_state.vr_state);
	}
	
	thread_state.textureCamera = textureCamera;
	EnqueueFutureResult(thread_state, textureCamera, options);
	AddFrame(textureCamera);
}


void DynamicTexture::EnqueueFutureResult(ThreadState thread_state, InputCamera textureCamera, Options options)
{
	future_result = pool->enqueue([](ThreadState thread_state, InputCamera textureCamera, Options options) -> ThreadState { 
		++thread_state.currentFrame;
		if (std::ranges::find(thread_state.i_frames, thread_state.currentFrame) != thread_state.i_frames.end() && textureCamera.nameColor != thread_state.textureCamera.nameColor)
		{
			thread_state.textureCamera = textureCamera;
			video_reader_close(&thread_state.vr_state);

			video_reader_open(&thread_state.vr_state, (textureCamera.jsonDirectory + textureCamera.nameColor).c_str());
			video_reader_seek_frame(&thread_state.vr_state, thread_state.currentFrame);
		}
		else
		{
			if (thread_state.currentFrame >= options.startFrame+thread_state.nrFrames+1)
			{
				thread_state.currentFrame = options.startFrame+1;
				video_reader_seek_frame(&thread_state.vr_state, thread_state.currentFrame);
			}
			else
			{
				video_reader_next_frame(&thread_state.vr_state);
			}
		}
		return thread_state;
	}, thread_state, textureCamera, options);
}

void DynamicTexture::AddFrame(InputCamera newTextureCamera)
{
	ThreadState thread_state = future_result.get();
	
	unsigned char* luminance = get_luminance(&thread_state.vr_state);
	unsigned char* chrominance = get_chrominance(&thread_state.vr_state);

	luminanceTexture.BindData(luminance);
	chrominanceTexture.BindData(chrominance);

	delete chrominance;

	DynamicTexture::textureCamera = thread_state.textureCamera;

	EnqueueFutureResult(thread_state, newTextureCamera, options);
}

void DynamicTexture::Export(Shader& shader, int i)
{
	shader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].model").c_str()), 1, GL_FALSE, glm::value_ptr(textureCamera.model));
	glUniform2fv(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].pp").c_str()), 1, glm::value_ptr(textureCamera.pp));
	glUniform2fv(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].focal").c_str()), 1, glm::value_ptr(textureCamera.intrinsics.focal));
	glUniform2f(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].res").c_str()), float(textureCamera.intrinsics.res[0]), float(textureCamera.intrinsics.res[1]));
	glUniform1f(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].z_far").c_str()), textureCamera.z_far);
	glUniform1f(glGetUniformLocation(shader.ID, ("cameras["+std::to_string(i)+"].z_near").c_str()), textureCamera.z_near);
}

void DynamicTexture::Bind()
{
	luminanceTexture.Bind();
	chrominanceTexture.Bind();
}

void DynamicTexture::Unbind()
{
	luminanceTexture.Unbind();
	chrominanceTexture.Unbind();
}

void DynamicTexture::Delete()
{
	if (future_result.valid())
	{
		ThreadState thread_state = future_result.get();
		video_reader_close(&thread_state.vr_state);
	}
	luminanceTexture.Delete();
	chrominanceTexture.Delete();
}