#pragma once

#include <customEngine.hpp>

class GameLayer : public CustomEngine::Layer
{
public:
	GameLayer() : Layer("GameLayer")
	{
		std::cout << "GameLayer created\n";
	}

	void onAttach() override
	{
		// Called when the layer is pushed onto the layer stack
	}

	void onDetach() override
	{
		// Called when the layer is popped from the layer stack
	}

	void onUpdate() override
	{
		// Called every frame
	}

	void onEvent(CustomEngine::Event& event) override
	{
		// Called when an event is sent to the layer
	}
};