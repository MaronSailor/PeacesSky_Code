#pragma once
#include "../engineCore/core.h"
#include "../eventSystem/applicationEvent.hpp"
#include "../eventSystem/keyEvent.hpp"
#include "../eventSystem/mouseEvent.hpp"
#include <string>
#include <functional>

namespace CustomEngine
{
	struct WindowProps
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowProps(const std::string& Title = "Peaces Skyland",
					unsigned int Width = 1280,
					unsigned int Height = 720) :
			title(Title), width(Width), height(Height) {}
	};

	using EventCallbackFunction = std::function<void(Event&)>;

	class Window
	{
	public:
		virtual ~Window() = default;

		virtual void onUpdate() = 0;

		virtual unsigned int getWidth() const = 0;
		virtual unsigned int getHeight() const = 0;

		virtual void setEventCallback(const EventCallbackFunction& callback) = 0;

		static Window* Create(const WindowProps& props = WindowProps());

		virtual bool onResize(unsigned int width, unsigned int height) = 0;

		virtual void setCursorLocked(bool locked) = 0;

		virtual void setFullscreen(bool fullscreen) = 0;
		virtual bool isFullscreen() const = 0;

		virtual void centerWindow() {}
		virtual void setRenderViewport(unsigned int renderW, unsigned int renderH) {}
		virtual unsigned int getRenderFboId() const { return 0; }
		virtual void beginRenderToFBO() {}
		virtual void endRenderToFBO() {}
		virtual void setVSync(bool enabled) {}
	};
}