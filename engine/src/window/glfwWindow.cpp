#include "glfwWindow.hpp"
#include <algorithm>
#include <iostream>

namespace CustomEngine
{
	GlfwWindow::GlfwWindow(const WindowProps& props)
	{
		init(props);
	}

	void GlfwWindow::init(const WindowProps& props)
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_Data.title = props.title;
		m_Data.width = props.width;
		m_Data.height = props.height;

		m_Window = glfwCreateWindow(m_Data.width, m_Data.height, props.title.c_str(), NULL, NULL);
		if (!m_Window) glfwTerminate();

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(0); // vsync off by default

		if (glewInit() != GLEW_OK) std::cout << "glew error\n";
		std::cout << glGetString(GL_VERSION) << std::endl;

		checkOGL(glEnable(GL_DEPTH_TEST));
		checkOGL(glDepthFunc(GL_LESS));

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		checkOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		checkOGL(glEnable(GL_BLEND));

		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.width = width;
				data.height = height;

				WindowResizeEvent event(width, height);
				data.eventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.eventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch(action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.eventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.eventCallback(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.eventCallback(event);
						break;
					}
				}
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.eventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.eventCallback(event);
						break;
					}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.eventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPosition, double yPosition)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPosition, (float)yPosition);
				data.eventCallback(event);
				});

			}

			GlfwWindow::~GlfwWindow()
	{
		if (m_RenderFbo != 0)
		{
			glDeleteFramebuffers(1, &m_RenderFbo);
			glDeleteTextures(1, &m_RenderColorTex);
			glDeleteRenderbuffers(1, &m_RenderDepthRbo);
		}
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void GlfwWindow::onUpdate()
	{
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	bool GlfwWindow::onResize(unsigned int width, unsigned int height)
	{
		if (m_ResizeGuard)
		{
			m_Data.width  = width;
			m_Data.height = height;
			int fbW, fbH;
			glfwGetFramebufferSize(m_Window, &fbW, &fbH);
			if (fbW > 0 && fbH > 0) glViewport(0, 0, fbW, fbH);
			return true;
		}
		m_ResizeGuard = true;
		glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
		m_Data.width  = width;
		m_Data.height = height;
		int fbW, fbH;
		glfwGetFramebufferSize(m_Window, &fbW, &fbH);
		glViewport(0, 0, fbW > 0 ? fbW : static_cast<int>(width), fbH > 0 ? fbH : static_cast<int>(height));
		m_ResizeGuard = false;
		return true;
	}

	void GlfwWindow::setFullscreen(bool fullscreen)
	{
		if (fullscreen == m_IsFullscreen) return;
		m_ResizeGuard = true;
		m_IsFullscreen = fullscreen;
		if (fullscreen)
		{
			glfwGetWindowPos(m_Window, &m_WindowedPosX, &m_WindowedPosY);
			glfwGetWindowSize(m_Window, &m_WindowedWidth, &m_WindowedHeight);
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			int monX = 0, monY = 0;
			glfwGetMonitorPos(monitor, &monX, &monY);
			// Borderless windowed: no decoration, positioned at monitor origin at monitor size.
			// Avoids exclusive fullscreen so OS screenshot tools continue to work.
			glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(m_Window, nullptr, monX, monY, mode->width, mode->height, 0);
			m_Data.width  = static_cast<unsigned int>(mode->width);
			m_Data.height = static_cast<unsigned int>(mode->height);
		}
		else
		{
			glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(m_Window, nullptr, m_WindowedPosX, m_WindowedPosY, m_WindowedWidth, m_WindowedHeight, 0);
			m_Data.width  = static_cast<unsigned int>(m_WindowedWidth);
			m_Data.height = static_cast<unsigned int>(m_WindowedHeight);
		}
		m_ResizeGuard = false;
		int fbW, fbH;
		glfwGetFramebufferSize(m_Window, &fbW, &fbH);
		if (fbW > 0 && fbH > 0) glViewport(0, 0, fbW, fbH);
	}
	void GlfwWindow::setCursorLocked(bool locked)
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	void GlfwWindow::centerWindow()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (!monitor) return;
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (!mode) return;
		int monX = 0, monY = 0;
		glfwGetMonitorPos(monitor, &monX, &monY);
		const int wx = monX + (mode->width  - static_cast<int>(m_Data.width))  / 2;
		const int wy = monY + (mode->height - static_cast<int>(m_Data.height)) / 2;
		glfwSetWindowPos(m_Window, wx, wy);
	}

	void GlfwWindow::setRenderViewport(unsigned int renderW, unsigned int renderH)
	{
		// Destroy any existing FBO
		if (m_RenderFbo != 0)
		{
			glDeleteFramebuffers(1, &m_RenderFbo);
			glDeleteTextures(1, &m_RenderColorTex);
			glDeleteRenderbuffers(1, &m_RenderDepthRbo);
			m_RenderFbo = 0; m_RenderColorTex = 0; m_RenderDepthRbo = 0;
		}
		m_RenderW = 0; m_RenderH = 0;

		// 0,0 or matching window size → direct rendering, no FBO
		if (renderW == 0 || renderH == 0 ||
			(renderW == m_Data.width && renderH == m_Data.height))
			return;

		m_RenderW = renderW;
		m_RenderH = renderH;

		glGenFramebuffers(1, &m_RenderFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RenderFbo);

		glGenTextures(1, &m_RenderColorTex);
		glBindTexture(GL_TEXTURE_2D, m_RenderColorTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(renderW), static_cast<int>(renderH), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderColorTex, 0);

		glGenRenderbuffers(1, &m_RenderDepthRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RenderDepthRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(renderW), static_cast<int>(renderH));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderDepthRbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GlfwWindow::beginRenderToFBO()
	{
		if (m_RenderFbo != 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_RenderFbo);
			glViewport(0, 0, static_cast<int>(m_RenderW), static_cast<int>(m_RenderH));
		}
	}

	void GlfwWindow::endRenderToFBO()
	{
		if (m_RenderFbo != 0)
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RenderFbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(
				0, 0, static_cast<int>(m_RenderW), static_cast<int>(m_RenderH),
				0, 0, static_cast<int>(m_Data.width), static_cast<int>(m_Data.height),
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Restore full viewport for ImGui
			glViewport(0, 0, static_cast<int>(m_Data.width), static_cast<int>(m_Data.height));
		}
	}

	void GlfwWindow::setVSync(bool enabled)
	{
		glfwSwapInterval(enabled ? 1 : 0);
	}
}