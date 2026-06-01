#include "pch.h"
#include "imguiLayer.hpp"

#include "../engineCore/application.hpp"
#include "../window/glfwWindow.hpp"


namespace CustomEngine
{
	ImGuiLayer::ImGuiLayer(Application& application)
		: Layer("ImGuiLayer"), m_Application(application)
	{
	}

	void ImGuiLayer::onAttach()
	{
		IMGUI_CHECKVERSION();
		m_Context = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_Context);
		ImGui::StyleColorsDark();

		auto* glfwWindow = dynamic_cast<GlfwWindow*>(&m_Application.getWindow());
		if (!glfwWindow || !glfwWindow->getWindowPtr())
		{
			ImGui::DestroyContext(m_Context);
			m_Context = nullptr;
			return;
		}

		if (!ImGui_ImplGlfw_InitForOpenGL(glfwWindow->getWindowPtr(), true))
		{
			ImGui::DestroyContext(m_Context);
			m_Context = nullptr;
			return;
		}

		if (!ImGui_ImplOpenGL3_Init(m_GlslVersion))
		{
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext(m_Context);
			m_Context = nullptr;
			return;
		}

		m_Initialized = true;
	}

	void ImGuiLayer::onDetach()
	{
		if (!m_Initialized) return;

		ImGui::SetCurrentContext(m_Context);
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext(m_Context);
		m_Context = nullptr;
		m_Initialized = false;
	}

	bool ImGuiLayer::begin()
	{
		if (!m_Initialized) return false;
		ImGui::SetCurrentContext(m_Context);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		return true;
	}

	void ImGuiLayer::end()
	{
		if (!m_Initialized) return;

		ImGui::SetCurrentContext(m_Context);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}