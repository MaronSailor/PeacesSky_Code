#include "vertexBuffer.hpp"

#include "../helper/OGLImport.hpp"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	checkOGL(glGenBuffers(1, &m_RendererId));
	checkOGL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
	checkOGL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(VertexBuffer&& other)
{
	m_RendererId = other.m_RendererId;
	other.m_RendererId = 0;
}

void VertexBuffer::bind() const
{
	checkOGL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
}

void VertexBuffer::unbind() const
{
	checkOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::setData(const void* data, unsigned int size)
{
	checkOGL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
	checkOGL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW));
}

void VertexBuffer::destroy()
{
	checkOGL(glDeleteBuffers(1, &m_RendererId));
}