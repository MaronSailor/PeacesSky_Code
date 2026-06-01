#include <customEngine.hpp>

#include <iostream>

#include "cloudSystem.hpp"

class WorldLayer : public CustomEngine::Layer
{
	CustomEngine::Application* app;

	Scene& scene;

	CloudSystem m_clouds;

	CustomEngine::EngineGui::GeneralSettings* m_generalSettings = nullptr;

public:
	WorldLayer(CustomEngine::Application* application, CustomEngine::EngineGui::GeneralSettings& generalSettings)
		: Layer("worldLayer"), app(application), scene(app->getSceneManager().getScene(0)), m_generalSettings(&generalSettings)
	{
		std::cout << "running Sandbox worldLayer\n";

		unsigned int SpaceEntity = scene.addEntity();
		Transform spaceTransform;
		RenderComponent spaceRender;
		spaceRender.setResourceLoader(app->getResourceloaderPtr());

		scene.entities[SpaceEntity].addComponent(spaceTransform);
		scene.entities[SpaceEntity].selectComponent<Transform>().scale = { 10000.0, 10000.0, 10000.0 };
		scene.entities[SpaceEntity].addComponent(spaceRender);
		scene.entities[SpaceEntity].selectComponent<RenderComponent>().setMesh("resources/cylinder.obj");
		scene.entities[SpaceEntity].selectComponent<RenderComponent>().setMaterial("resources/sky.jpeg");
		scene.entities[SpaceEntity].selectComponent<RenderComponent>().setTransformIndex(scene.entities[SpaceEntity].getComponentIndex<Transform>());




		unsigned int Entity3 = scene.addEntity();
		Transform transformC;
		LightSource lightsourceA;
		lightsourceA.location = { 100, 500, 0 };
		lightsourceA.rotation = { 75, 0, 0 };
		lightsourceA.type = DirectionalLight;
		lightsourceA.color = { 1.0, 1.0, 1.0, 1.0 };
		scene.entities[Entity3].addComponent(lightsourceA);

		//unsigned int Entity4 = scene.addEntity(); // only one light supported for now

		m_clouds.init(app, scene);
	}


	void onUpdate() override
	{
		float dT = app->getDeltaTIme() / 10.0f;
		float drawDist = m_generalSettings ? m_generalSettings->cloudDrawDistance : 9500.0f;
		m_clouds.update(dT, drawDist, scene.activeCamera->location);
	}
};
