#pragma once
#include <string>
#include "../../resourceLoader/materialData.hpp"

class Texture
{
	unsigned int m_RendererId;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

public:
	Texture(MaterialData& material);

	Texture(Texture&& other);

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	void destroy();

	inline int getWidth() const { return m_Width; }
	inline int getHeight() const { return m_Height; }
};