#include "event.hpp"
#include <sstream>

namespace CustomEngine
{
	class CUSTOMENGINE_API KeyEvent : public Event
	{
	protected:
		KeyEvent(int keyCode) : m_KeyCode(keyCode) {}

		int m_KeyCode;

	public:
		inline int getKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	};

	class CUSTOMENGINE_API KeyPressedEvent : public KeyEvent
	{
		int m_RepeatCount;

	public:
		KeyPressedEvent(int keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return stringStream.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	};

	class CUSTOMENGINE_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}

		std::string toString() const override
		{
			std::stringstream stringStream;
			stringStream << "KeyReleasedEvent: " << m_KeyCode;
			return stringStream.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}