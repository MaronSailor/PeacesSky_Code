#include <customEngine.hpp>

#include <iostream>

class ValidationLayer : public CustomEngine::Layer
{
	CustomEngine::Application* app;
public:
	ValidationLayer(CustomEngine::Application* application) : Layer("validationLayer"), app(application)
	{
	}

	void onUpdate() override
	{
	}
};