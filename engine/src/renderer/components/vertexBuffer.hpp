#pragma once

class VertexBuffer
{
	unsigned int m_RendererId;

public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(VertexBuffer&& other);

	void bind() const;
	void unbind() const;

	void setData(const void* data, unsigned int size);

	void destroy();
};