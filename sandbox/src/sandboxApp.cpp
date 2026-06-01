#include <customEngine.hpp>

#include "setupLayer.hpp"
#include "worldLayer.hpp"
#include "gameLayer.hpp"

class Sandbox : public CustomEngine::Application
{

public:
	Sandbox()
	{
		auto* setup = new SetupLayer(this);
		pushLayer(setup);
		pushLayer(new WorldLayer(this, setup->getGeneralSettings()));
	}

	~Sandbox()
	{

	}
};

CustomEngine::Application* CustomEngine::CreateApplication()
{
	return new Sandbox();
}