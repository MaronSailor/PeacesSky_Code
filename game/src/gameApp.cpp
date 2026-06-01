#include <customEngine.hpp>

class Game : public CustomEngine::Application
{
public:
	Game()
	{

	}

	~Game()
	{

	}
};

CustomEngine::Application* CustomEngine::CreateApplication()
{
	return new Game();
}