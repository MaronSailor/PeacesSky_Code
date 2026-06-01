#include "imageBuffer.hpp"
#include "../helper/OGLImport.hpp"

void ImageBuffer::create(unsigned int width, unsigned int height)
{
	checkOGL(glGenFramebuffers(1, &m_RendererId));
	checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));

	checkOGL(glGenTextures(1, &m_BufferTexture));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_BufferTexture));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	checkOGL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	checkOGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

	checkOGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BufferTexture, 0));
	

	checkOGL(glGenTextures(1, &m_DepthTexture));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_DepthTexture));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	checkOGL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	checkOGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr));
	checkOGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0));
	
	GLenum bufferAttachments[1] = { GL_COLOR_ATTACHMENT0 };
	checkOGL(glDrawBuffers(1, bufferAttachments));

	checkOGL(glBindTexture(GL_TEXTURE_2D, 0));
	checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

ImageBuffer::ImageBuffer(unsigned int width, unsigned int height) : m_Width(width), m_Height(height)
{
	create(width, height);
}

void ImageBuffer::bind()
{
	checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
}

void ImageBuffer::bindTextures(unsigned int colorTextureSlot, unsigned int depthTextureSlot)
{
	checkOGL(glActiveTexture(GL_TEXTURE0 + colorTextureSlot));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_BufferTexture));
	checkOGL(glActiveTexture(GL_TEXTURE0 + depthTextureSlot));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_DepthTexture));
}

void ImageBuffer::bindToScreen(unsigned int screenWidth, unsigned int screenHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void ImageBuffer::unbind()
{
	checkOGL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
	checkOGL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
	checkOGL(glBindTexture(GL_TEXTURE_2D, 0));
}

void ImageBuffer::destroy()
{
	checkOGL(glDeleteFramebuffers(1, &m_RendererId));
	checkOGL(glDeleteTextures(1, &m_BufferTexture));
	checkOGL(glDeleteTextures(1, &m_DepthTexture));
}

void ImageBuffer::resize(unsigned int width, unsigned int height)
{
	destroy();
	m_Width = width;
	m_Height = height;
	create(width, height);
}