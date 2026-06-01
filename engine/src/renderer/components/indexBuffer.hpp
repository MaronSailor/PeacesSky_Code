#pragma once
class IndexBuffer
{
	unsigned int m_RendererId;
	unsigned int m_Count;

public:
	IndexBuffer(const unsigned int* data, unsigned int count);

	IndexBuffer(IndexBuffer&& other);

	void bind() const;
	void unbind() const;

	inline unsigned int getCount() const
	{
		return m_Count;
	}

	void setData(const unsigned int* data, unsigned int count);

	void destroy();
};