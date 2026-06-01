#pragma once

class ShadowMap
{
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_RendererId;
	unsigned int m_ShadowMap;

public:
	ShadowMap(unsigned int width, unsigned int height);
	ShadowMap(ShadowMap&& other);
	void bind();
	void unbind();
	void destroy();
	void bindMapTexture(unsigned int slot);
	const unsigned int getWidth() const;
	const unsigned int getHeight() const;

};