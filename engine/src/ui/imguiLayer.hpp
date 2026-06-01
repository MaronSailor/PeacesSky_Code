#pragma once

#include "../engineCore/layer.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

struct ImGuiContext;

namespace CustomEngine
{
	class Application;

	class ImGuiLayer : public Layer
	{
	public:
		explicit ImGuiLayer(Application& application);
		~ImGuiLayer() override = default;

		void onAttach() override;
		void onDetach() override;

		bool begin();
		void end();

		bool isInitialized() const { return m_Initialized; }

	private:
		Application& m_Application;
		const char* m_GlslVersion = "#version 150";
		ImGuiContext* m_Context = nullptr;
		bool m_Initialized = false;
	};
}