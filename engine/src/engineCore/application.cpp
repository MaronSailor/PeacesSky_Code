#include "application.hpp"

#include <iostream>
#include "../ecs/components/renderComponent.hpp"

#include "../ui/imguiLayer.hpp"

namespace CustomEngine
{

	Application::Application() : m_isRunning(true)
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
		m_RenderUnit = std::unique_ptr<RenderUnit>(RenderUnit::createRenderer());
		m_ResourceLoader = std::unique_ptr<ResourceLoaderUnit>(ResourceLoaderUnit::create());
		m_PhysicsEngine = std::unique_ptr<PhysicsUnit>(PhysicsUnit::createPhysicsUnit());

		m_ImGuiLayer = new ImGuiLayer(*this);
		pushOverlay(m_ImGuiLayer);
		m_ImGuiLayer->onAttach();
	}

	Application::~Application()
	{	
		if (m_ImGuiLayer)
		{
			m_ImGuiLayer->onDetach();
		}
	}

	void Application::onEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
		dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::onWindowResize, this, std::placeholders::_1));
		dispatcher.dispatch<MouseMovedEvent>(std::bind(&Application::onMousePositionChange, this, std::placeholders::_1));
		dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&Application::onMouseButtonPressed, this, std::placeholders::_1));
		dispatcher.dispatch<MouseButtonReleasedEvent>(std::bind(&Application::onMouseButtonReleased, this, std::placeholders::_1));
		dispatcher.dispatch<MouseScrolledEvent>(std::bind(&Application::onMouseScroll, this, std::placeholders::_1));
		dispatcher.dispatch<KeyPressedEvent>(std::bind(&Application::onKeyPressed, this, std::placeholders::_1));
		dispatcher.dispatch<KeyReleasedEvent>(std::bind(&Application::onKeyReleased, this, std::placeholders::_1));
	}

	void Application::run()
	{
		m_FrameTimer.start();
		while (m_isRunning)
		{
				// wait for previous frame physics
				m_PhysicsEngine->waitForCalculation();

				m_FrameTimer.update();
				m_ActiveSceneInfo.deltaTime = m_FrameTimer.getDeltaTime();

				for (Layer* layer : m_LayerStack) layer->onUpdate();

				m_Window->beginRenderToFBO();
				m_RenderUnit->renderScene(&m_ActiveSceneInfo);
				m_Window->endRenderToFBO();

				// kick physics thread while imgui renders
				m_PhysicsEngine->startAsyncCalculation(&m_ActiveSceneInfo);

				if (m_ImGuiLayer && m_ImGuiLayer->isInitialized())
				{
					m_ImGuiLayer->begin();
					for (Layer* layer : m_LayerStack) layer->onImGuiRender();
					m_ImGuiLayer->end();
				}

				m_Window->onUpdate();
			}

			// drain last physics job before cleanup
			m_PhysicsEngine->waitForCalculation();
	}

	bool Application::onWindowClose(WindowCloseEvent& event)
	{
		terminate();
		return true;
	}

	bool Application::onWindowResize(WindowResizeEvent& event)
	{
		m_Window->onResize(event.getWidth(), event.getHeight());
		m_RenderUnit->setWindowAspect(event.getWidth(), event.getHeight());
		return true;
	}

	bool Application::onKeyPressed(KeyPressedEvent& event)
	{
		m_InputHandler.updateKey(event.getKeyCode(), 1);
		return true;
	}

	bool Application::onKeyReleased(KeyReleasedEvent& event)
	{
		m_InputHandler.updateKey(event.getKeyCode(), 0);
		return true;
	}

	bool Application::onMouseButtonPressed(MouseButtonPressedEvent& event)
	{
		m_InputHandler.updateMouseButton(event.getMouseButton(), 1);
		return true;
	}

	bool Application::onMouseButtonReleased(MouseButtonReleasedEvent& event)
	{
		m_InputHandler.updateMouseButton(event.getMouseButton(), 0);
		return true;
	}

	bool Application::onMousePositionChange(MouseMovedEvent& event)
	{
		m_InputHandler.updateMousePosition(event.getX(), event.getY());
		return true;
	}

	bool Application::onMouseScroll(MouseScrolledEvent& event)
	{
		m_InputHandler.updateScrollWheel(event.getYOffset());
		return true;
	}

	void Application::resizeWindow(unsigned int width, unsigned int height)
	{
		m_Window->onResize(width, height);
		m_RenderUnit->setWindowAspect(width, height);
	}

	void Application::setWindowFullscreen(bool fullscreen)
	{
		m_Window->setFullscreen(fullscreen);
		m_RenderUnit->setWindowAspect(m_Window->getWidth(), m_Window->getHeight());
	}

	void Application::centerWindow()
	{
		m_Window->centerWindow();
	}

	void Application::setRenderResolution(unsigned int width, unsigned int height)
	{
		m_Window->setRenderViewport(width, height);
		const unsigned int aspectW = (width  > 0) ? width  : m_Window->getWidth();
		const unsigned int aspectH = (height > 0) ? height : m_Window->getHeight();
		m_RenderUnit->setWindowAspect(aspectW, aspectH);
		m_RenderUnit->setOutputFbo(m_Window->getRenderFboId());
	}

	void Application::terminate()
	{
		m_isRunning = false;
	}

	void Application::pushLayer(Layer* layer)
	{
		m_LayerStack.pushLayer(layer);
	}

	void Application::pushOverlay(Layer* layer)
	{
		m_LayerStack.pushOverlay(layer);
	}

	SceneManager& Application::getSceneManager()
	{
		return m_SceneManager;
	}

	ResourceLoader* Application::getResourceloaderPtr()
	{
		return (ResourceLoader*)&*m_ResourceLoader;
	}

	InputHandler& Application::getInputHandler()
	{
		return m_InputHandler;
	}

	void Application::setActiveSceneInfo(EngineSceneInfo& sceneInfo)
	{
		m_ActiveSceneInfo = sceneInfo;
	}

	Aspect Application::getWindowAspect()
	{
		return { (*m_Window).getWidth(), (*m_Window).getHeight()};
	}
}