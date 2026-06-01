#include "shadowMap.hpp"
#include "../helper/OGLImport.hpp"

ShadowMap::ShadowMap(unsigned int width, unsigned int height) : m_Width(width), m_Height(height)
{
	checkOGL(glGenFramebuffers(1, &m_RendererId));

	checkOGL(glGenTextures(1, &m_ShadowMap));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_ShadowMap));

	checkOGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	checkOGL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor));

	checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
	checkOGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap, 0));
	checkOGL(glDrawBuffer(GL_NONE));
	checkOGL(glReadBuffer(GL_NONE));

	unbind();
}

ShadowMap::ShadowMap(ShadowMap&& other)
{
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_RendererId = other.m_RendererId;
	m_ShadowMap = other.m_ShadowMap;

	other.m_RendererId = 0;
	other.m_ShadowMap = 0;
}

void ShadowMap::bind()
{
	checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
}

void ShadowMap::unbind()
{
	checkOGL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
	checkOGL(glBindTexture(GL_TEXTURE_2D, 0));
}

void ShadowMap::destroy()
{
	checkOGL(glDeleteFramebuffers(1, &m_RendererId));
	checkOGL(glDeleteTextures(1, &m_ShadowMap));
}

void ShadowMap::bindMapTexture(unsigned int slot)
{
	checkOGL(glActiveTexture(GL_TEXTURE0 + slot));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_ShadowMap));
}

const unsigned int ShadowMap::getWidth() const
{
	return m_Width;
}

const unsigned int ShadowMap::getHeight() const
{
	return m_Height;
}