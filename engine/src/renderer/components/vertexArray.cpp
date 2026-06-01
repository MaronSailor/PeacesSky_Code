#include "vertexArray.hpp"

VertexArray::VertexArray()
{
	checkOGL(glGenVertexArrays(1, &m_RendererId));
}

VertexArray::~VertexArray()
{
	checkOGL(glDeleteVertexArrays(1, &m_RendererId));
}

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	bind();
	vb.bind();

	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		checkOGL(glEnableVertexAttribArray(i));
		checkOGL(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
	}
}

void VertexArray::bind() const
{
	checkOGL(glBindVertexArray(m_RendererId));
}

void VertexArray::unbind() const
{
	checkOGL(glBindVertexArray(0));
}