#pragma once
#include "vertexBuffer.hpp"
#include "vertexBufferLayout.hpp"

class VertexArray
{
	unsigned int m_RendererId;

public:
	VertexArray();
	~VertexArray();

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void bind() const;
	void unbind() const;
};