#pragma once

#include "core.h"
#include "../eventSystem/event.hpp"

namespace CustomEngine
{
	class CUSTOMENGINE_API Layer
	{
	protected:
		std::string m_DebugName;
	public:
		Layer(const std::string & name = "Layer");
		virtual ~Layer();

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(Event& event) {}
		virtual void onImGuiRender() {}

		inline const std::string& getName() const { return m_DebugName; }
	};
}