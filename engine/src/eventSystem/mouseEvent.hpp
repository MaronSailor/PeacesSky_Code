#include "../engineCore/core.h"
#include "event.hpp"
#include <sstream>

namespace CustomEngine
{
	class CUSTOMENGINE_API MouseMovedEvent : public Event
	{
		float m_MouseX, m_MouseY;

	public:
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		inline float getX() const { return m_MouseX; }
		inline float getY() const { return m_MouseY; }

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return stringStream.str();
		}
	
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	};

	class CUSTOMENGINE_API MouseScrolledEvent : public Event
	{
		float m_XOffset, m_YOffset;

	public:
		MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline float getXOffset() const { return m_XOffset; }
		inline float getYOffset() const { return m_YOffset; }

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return stringStream.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	};

	class CUSTOMENGINE_API MouseButtonEvent : public Event
	{
	protected:
		int m_Button;

		MouseButtonEvent(int button) : m_Button(button) {}

	public:
		inline int getMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	};

	class CUSTOMENGINE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "MouseButtonPressedEvent: " << m_Button;
			return stringStream.str();
		}
		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class CUSTOMENGINE_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "MouseButtonReleasedEvent: " << m_Button;
			return stringStream.str();
		}
		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}