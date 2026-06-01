#include "texture.hpp"

#include "../helper/OGLImport.hpp"
#include <stb_image.h>

Texture::Texture(MaterialData& material)
	: m_RendererId(0), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	m_LocalBuffer = material.textureData.data();
	m_Width = material.textureWidth;
	m_Height = material.textureHeight;
	m_BPP = material.textureBPP;

	checkOGL(glGenTextures(1, &m_RendererId));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_RendererId));

	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	checkOGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer)); // exeption when multiple textures
	checkOGL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(Texture&& other)
{
	m_FilePath = other.m_FilePath;
	m_LocalBuffer = other.m_LocalBuffer;
	m_RendererId = other.m_RendererId;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_BPP = other.m_BPP;

	other.m_RendererId = -1;
}

void Texture::destroy()
{
	checkOGL(glDeleteTextures(1, &m_RendererId));
}

void Texture::bind(unsigned int slot /*= 0*/) const
{
	checkOGL(glActiveTexture(GL_TEXTURE0 + slot));
	checkOGL(glBindTexture(GL_TEXTURE_2D, m_RendererId));
}

void Texture::unbind() const
{
	checkOGL(glBindTexture(GL_TEXTURE_2D, 0));
}