#pragma once

#include "core.h"
#include "../eventSystem/event.hpp"
#include "layerStack.hpp"
#include "../eventSystem/applicationEvent.hpp"

#include "../sceneManager/sceneManager.hpp"

#include "../renderer/components/renderUnit.hpp"
#include "../window/window.hpp"
#include "../resourceLoader/resourceLoaderUnit.hpp"

#include "../inputHandler/inputHandler.hpp"

#include "../ecs/engineSceneInfo.hpp"

#include "../physicsEngine/physicsUnit.hpp"

#include "../resourceLoader/Timer.hpp"

#include "../audio/audioEngine.hpp"

namespace CustomEngine
{
	class ImGuiLayer;

	class CUSTOMENGINE_API Application
	{
		LayerStack m_LayerStack;
		bool m_isRunning;

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<RenderUnit> m_RenderUnit;
		std::unique_ptr<PhysicsUnit> m_PhysicsEngine;

		EngineSceneInfo m_ActiveSceneInfo;

		std::unique_ptr<ResourceLoaderUnit> m_ResourceLoader;

		SceneManager m_SceneManager;
		InputHandler m_InputHandler;
		Timer m_FrameTimer;
		AudioEngine m_AudioEngine;

		ImGuiLayer* m_ImGuiLayer = nullptr;

		bool onWindowClose(WindowCloseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);

		bool onKeyPressed(KeyPressedEvent& event);
		bool onKeyReleased(KeyReleasedEvent& event);
		bool onMouseButtonPressed(MouseButtonPressedEvent& event);
		bool onMouseButtonReleased(MouseButtonReleasedEvent& event);
		bool onMousePositionChange(MouseMovedEvent& event);
		bool onMouseScroll(MouseScrolledEvent& event);

	public:
		Application();
		virtual ~Application();

		void run();

		void onEvent(Event& event);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);

		void terminate();

		SceneManager& getSceneManager();
		ResourceLoader* getResourceloaderPtr();
		InputHandler& getInputHandler();
		Aspect getWindowAspect();
		Window& getWindow() { return *m_Window; }

		void resizeWindow(unsigned int width, unsigned int height);
		void setWindowFullscreen(bool fullscreen);
		void centerWindow();
		void setRenderResolution(unsigned int width, unsigned int height);

		double getDeltaTIme() { return m_FrameTimer.getDeltaTime(); }
		AudioEngine& getAudioEngine() { return m_AudioEngine; }

		void setActiveSceneInfo(EngineSceneInfo& sceneInfo);
	};


	Application* CreateApplication(); // defined in client

}