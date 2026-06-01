#include <customEngine.hpp>

#include <iostream>
#include "../../engine/src/timer/Timer.hpp"

#define PI 3.1415926535f

int Iteration = 0;

class TestLayer : public CustomEngine::Layer
{
	CustomEngine::Application* app;
	bool mouseLookActive = false;

public:
	TestLayer(CustomEngine::Application* application) : Layer("testLayer"), app(application)
	{
		std::cout << "running Sandbox TestLayer\n";

		app->getSceneManager().addScene();
		Scene& scene = app->getSceneManager().getScene(0);

		EngineSceneInfo sceneInfo;
		sceneInfo.pRenderComponentStorage = scene.ecs.getComponentStorage<RenderComponent>();
		sceneInfo.pActiveCamera = scene.activeCamera;
		app->setActiveSceneInfo(sceneInfo);

		scene.entities.emplace_back(scene.ecs);
		Transform transformA;
		RenderComponent renderCA;
		renderCA.setResourceLoader(app->getResourceloaderPtr());

		scene.entities[0].addComponent(transformA);
		scene.entities[0].selectComponent<Transform>().location.z = 0;
		scene.entities[0].selectComponent<Transform>().location.y = -250;
		scene.entities[0].selectComponent<Transform>().scale = { 100,100,100 };
		scene.entities[0].addComponent(renderCA);
		scene.entities[0].selectComponent<RenderComponent>().setMesh("resources/moon.obj");
		scene.entities[0].selectComponent<RenderComponent>().setMaterial("resources/moon.png");


		scene.entities.emplace_back(scene.ecs);
		Transform transformB;
		RenderComponent renderCB;
		renderCB.setResourceLoader(app->getResourceloaderPtr());

		auto Space = scene.entities[1];

		Space.addComponent(transformB);
		Space.selectComponent<Transform>().location.x = 0;
		Space.selectComponent<Transform>().scale = { 1000000,1000000,1000000 };
		Space.addComponent(renderCB);
		Space.selectComponent<RenderComponent>().setMesh("resources/icosphere_space.obj");
		Space.selectComponent<RenderComponent>().setMaterial("resources/space3.png");


		scene.entities.emplace_back(scene.ecs);
		Transform transformC;
		RenderComponent renderCC;
		renderCC.setResourceLoader(app->getResourceloaderPtr());

		auto spaceShip = scene.entities[2];
		
		spaceShip.addComponent(transformC);
		spaceShip.selectComponent<Transform>().location = { 0,0,10 };
		spaceShip.addComponent(renderCC);
		spaceShip.selectComponent<RenderComponent>().setMesh("resources/MCCV-2.obj");
		spaceShip.selectComponent<RenderComponent>().setMaterial("resources/texture.png");
		
		
		
		
		scene.sceneCamera.farP = 10000000;

		
		
		scene.entities[0].selectComponent<RenderComponent>().setTransform(scene.entities[0].selectComponent<Transform>()); // needs to be called after all tranform adds
		Space.selectComponent<RenderComponent>().setTransform(scene.entities[1].selectComponent<Transform>());
		spaceShip.selectComponent<RenderComponent>().setTransform(scene.entities[2].selectComponent<Transform>());
	}

	void onUpdate() override
	{
		Scene& scene = app->getSceneManager().getScene(0);

		InputHandler& inputHandler = app->getInputHandler();
		
		if (inputHandler.mouse.rMouseButton && !mouseLookActive)
		{
			mouseLookActive = true;
			app->getWindow()->setCursorLocked(true);
			app->getWindow()->centerCursor();

			//scene.cameraController.state.initialized = false;
		}
		else if (!inputHandler.mouse.rMouseButton && mouseLookActive)
		{
			mouseLookActive = false;
			app->getWindow()->setCursorLocked(false);
		}

		if (inputHandler.mouse.rMouseButton && mouseLookActive)
		{
			unsigned int width = app->getWindow()->getWidth();
			unsigned int height = app->getWindow()->getHeight();

			Aspect windowCenter = { (width / 2), (height / 2) };

			float moveSpeed = 0.25f;
			float fwdInput   = (inputHandler.keys.w ? moveSpeed : 0.0f) - (inputHandler.keys.s ? moveSpeed : 0.0f);
			float rightInput = (inputHandler.keys.d ? moveSpeed : 0.0f) - (inputHandler.keys.a ? moveSpeed : 0.0f);
			float upInput    = (inputHandler.keys.space ? moveSpeed : 0.0f) - (inputHandler.keys.ctrl ? moveSpeed : 0.0f);

			Quaternion camRotationQuat;

			scene.cameraController.update(
				inputHandler.mouse.x, 
				inputHandler.mouse.y, 
				fwdInput,
				rightInput,
				upInput,
				camRotationQuat,              
				scene.entities.at(2).selectComponent<Transform>().location,
				windowCenter,
				true							// useCenterLock
			);

			//scene.activeCamera->rotation.y = scene.cameraController.yawDeg;
			//scene.activeCamera->rotation.x = scene.cameraController.pitchDeg;

			std::cout << "Pitch: " << scene.cameraController.getPitchDeg() << " Yaw: " << scene.cameraController.getYawDeg() << "\n";
			std::cout << "Ship Rot X: " << scene.entities.at(2).selectComponent<Transform>().rotation.x << " Rot Y: " << scene.entities.at(2).selectComponent<Transform>().rotation.y << "\n";
			
			scene.activeCamera->rotation.x = scene.cameraController.getPitchDeg();
			scene.activeCamera->rotation.y = scene.cameraController.getYawDeg();

			scene.entities.at(2).selectComponent<Transform>().rotation.y = scene.activeCamera->rotation.y - 180;
			scene.entities.at(2).selectComponent<Transform>().rotation.x = scene.activeCamera->rotation.x;

			app->getWindow()->centerCursor();
			inputHandler.mouse.x = windowCenter.x;
			inputHandler.mouse.y = windowCenter.y;
		}
		//scene.entities.at(2).selectComponent<Transform>().location = scene.activeCamera->location + Vec3{ 0, -75.0f, 50.0f };
		scene.activeCamera->location = scene.entities.at(2).selectComponent<Transform>().location + Vec3{ 0, 75.0f, 50.0f };
		scene.entities.at(2).selectComponent<Transform>().rotation.y += inputHandler.keys.q;
		scene.entities.at(2).selectComponent<Transform>().rotation.y -= inputHandler.keys.e;
		scene.entities.at(2).selectComponent<Transform>().rotation.x += inputHandler.keys.r;
		scene.entities.at(2).selectComponent<Transform>().rotation.x -= inputHandler.keys.f;
		scene.entities.at(2).selectComponent<Transform>().rotation.z += inputHandler.keys.x;
		scene.entities.at(2).selectComponent<Transform>().rotation.z -= inputHandler.keys.c;

		// Re-assign transform reference to RenderComponent to ensure pointer validity

		if (inputHandler.keys.esc) app->terminate();
		
		inputHandler.beginFrame();
	}
};