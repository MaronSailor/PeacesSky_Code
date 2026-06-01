class ImageBuffer
{
private:
	unsigned int m_RendererId;
	unsigned int m_BufferTexture;
	unsigned int m_DepthTexture;

	unsigned int m_Width;
	unsigned int m_Height;

private:
	void create(unsigned int width, unsigned int height);

public:
	ImageBuffer(unsigned int width, unsigned int height);
	void bind();
	void bindTextures(unsigned int colorTextureSlot, unsigned int depthTextureSlot);
	void bindToScreen(unsigned int screenWidth, unsigned int screenHeight);
	void unbind();
	void destroy();
	void resize(unsigned int width, unsigned int height);
};