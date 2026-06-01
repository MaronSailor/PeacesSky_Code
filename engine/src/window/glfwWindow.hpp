#include "../renderer/helper/OGLImport.hpp"
#include "window.hpp"

namespace CustomEngine
{
	class GlfwWindow : public Window
	{
		struct WindowData
		{
			std::string title = "";
			unsigned int width = 0, height = 0;
			EventCallbackFunction eventCallback;
		};


		GLFWwindow* m_Window;
		WindowData m_Data;

		bool m_IsFullscreen = false;
		bool m_ResizeGuard  = false;
		int  m_WindowedPosX = 100, m_WindowedPosY = 100;
		int  m_WindowedWidth = 1280, m_WindowedHeight = 720;

		// FBO for render-resolution scaling in fullscreen
		GLuint       m_RenderFbo      = 0;
		GLuint       m_RenderColorTex = 0;
		GLuint       m_RenderDepthRbo = 0;
		unsigned int m_RenderW        = 0;
		unsigned int m_RenderH        = 0;

	public:

		GlfwWindow(const WindowProps& props);

		virtual ~GlfwWindow();

		void onUpdate();

		inline GLFWwindow* getWindowPtr() const { return m_Window; }


		void setEventCallback(const EventCallbackFunction& callback) override { m_Data.eventCallback = callback; }
		void init(const WindowProps& props);

		unsigned int getWidth() const override {return m_Data.width;}
		unsigned int getHeight() const override {return m_Data.height;}

		bool onResize(unsigned int width, unsigned int height) override;

		void setCursorLocked(bool locked);

		void setFullscreen(bool fullscreen) override;
		bool isFullscreen() const override { return m_IsFullscreen; }

		void centerWindow() override;
		void setRenderViewport(unsigned int renderW, unsigned int renderH) override;
		unsigned int getRenderFboId() const override { return m_RenderFbo; }
		void beginRenderToFBO() override;
		void endRenderToFBO() override;
		void setVSync(bool enabled) override;
	};
}