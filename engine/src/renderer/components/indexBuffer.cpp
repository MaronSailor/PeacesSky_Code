#include "indexBuffer.hpp"

#include "../helper/OGLImport.hpp"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_Count(count)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	checkOGL(glGenBuffers(1, &m_RendererId));
	checkOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
	checkOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(IndexBuffer&& other)
{
	m_RendererId = other.m_RendererId;
	m_Count = other.m_Count;

	other.m_RendererId = 0;
}

void IndexBuffer::bind() const
{
	checkOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
}

void IndexBuffer::unbind() const
{
	checkOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::setData(const unsigned int* data, unsigned int count)
{
	m_Count = count;
	checkOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
	checkOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_DYNAMIC_DRAW));
}

void IndexBuffer::destroy()
{
	checkOGL(glDeleteBuffers(1, &m_RendererId));
}