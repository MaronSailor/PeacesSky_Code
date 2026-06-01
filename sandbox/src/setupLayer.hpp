#include <customEngine.hpp>
#include <random>

#include "ui/engineGui.hpp"

#include "cameraController.hpp"
#include "playerSpecs.hpp"
#include "blockManager.hpp"
#include "menuScene.hpp"
#include "mainMenu.hpp"
#include "gameVersion.hpp"
#include "net/netPackets.hpp"
#include "net/tcpSocket.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <string>

// ─── Gameplay tunables ────────────────────────────────────────────────────────
// Movement
float movementMultiplier  = 0.5f;   // base movement scale (modified by shift at runtime)
float moveSpeed           = 5.0f;   // base movement speed fed into PlayerSpecs
float flySpeedHorizontal  = 1.0f;   // horizontal speed multiplier while in fly mode
float flySpeedVertical    = 0.5f; // space/ctrl vertical speed in fly mode (units per dT unit)

float jetMaxSpeed         = 0.3f;   // terminal upward speed for jet propulsion
float jetDecay            = 0.02f;  // deceleration per dT unit when jet is released

// Ball
float ballSpeed           = 3.0f;   // projectile launch speed (world units)

// Camera
float cameraHeightOffset  = 4.0f;   // camera raised above player origin (1 block = 4 units)

// Platform / world (used before BlockManager is constructed; mirrored into it on startGame)
int   defaultPlatformSize        = 6;    // NxN footprint in blocks
int   defaultPlatformOffsetBlocks = 50;  // horizontal separation between the two platforms
	   // total build-height grid (base + build layers)

bool flyMode = false;
// ─────────────────────────────────────────────────────────────────────────────

class SetupLayer : public CustomEngine::Layer
{
	CustomEngine::Application* app;

	CameraController cameraController;

	Scene& scene;
	InputHandler& inputHandler;

	PlayerSpecs playerSpecs;
	BlockManager blockManager;

	uint32_t playerEntity = 0;
	uint32_t remotePlayerEntity = 0;

	bool wasVPressed = false;
	bool wasLPressed = false;
	bool wasRPressed = false;
	bool wasRightMousePressed = false;
	bool wasSpacePressed = false;
	bool wasEscPressed = false;
	bool isBuildMode = true;
	bool isFollowingBall = false;
	bool wasCPressed = false;
	uint32_t followedBallEntity = 0;

	bool isGameStarted = false;
	bool isMultiplayer = false;
	bool isHost = false;
	bool hasRemotePlayer = false;
	float reconnectTimer = 0.0f;
	float networkSendAccumulator = 0.0f;
	MainMenu mainMenu;

	std::string serverAddress;
	uint16_t serverPort = 28000;

	// Core 3: all TCP I/O happens on the network background thread
	NetworkThread m_net;

	bool pendingWorldInit = false;
	bool worldSeedSent = false;
	uint32_t worldSeed = 0;

	bool waitingForClient = false;
	bool connectingToServer = false;
	std::chrono::steady_clock::time_point connectStartTime{};
	static constexpr float maxConnectTimeoutSeconds = 15.0f;
	std::string connectionError;

	// add alongside the other network state members
	bool versionHandshakeSent = false;
	bool versionConfirmed = false;
	bool pendingVersionReject = false;
	float versionRejectTimer = -1.0f;

	bool isEscapeMenuOpen = false;

	bool pendingWindowChange = false;

	float matchTimer = 5.0f; // in minutes
	float matchDuration = 5.0f; // in minutes
	int minBlocksForLoss = 10; 
	bool matchEnded = false;
	bool localPlayerWon = false;

	float lastDeltaTime = 0.0f; // used for timing-sensitive operations that need to know the current frame's dT
		
	MenuScene m_menuScene;

public:
	CustomEngine::EngineGui::GeneralSettings& getGeneralSettings() { return mainMenu.getGeneralSettings(); }

	SetupLayer(CustomEngine::Application* application)
		: Layer("setupLayer")
		, app(application)
		, scene(app->getSceneManager().getScene(app->getSceneManager().addScene()))
		, inputHandler(app->getInputHandler())
		, playerSpecs()
		, blockManager(app, scene, inputHandler, playerSpecs)
	{
		std::cout << "running Sandbox setupLayer\n";

		// Apply tunables to PlayerSpecs
		playerSpecs.movementSpeed  = moveSpeed;
		playerSpecs.maxJetSpeed    = jetMaxSpeed;
		playerSpecs.jetDecay       = jetDecay;

		blockManager.setBallSpeed(ballSpeed);

		app->getAudioEngine().loadSound("jetpack",      "resources/jetpack.mp3");
		app->getAudioEngine().loadSound("ball_throw",   "resources/ball_throw.mp3");
		app->getAudioEngine().loadSound("ball_hit",     "resources/ball_hit.mp3");
		app->getAudioEngine().loadSound("block_place",  "resources/block_place.mp3");
		app->getAudioEngine().loadSound("block_remove", "resources/block_remove.mp3");
		app->getAudioEngine().loadSound("button_click", "resources/button_click.mp3");

		auto& gs = mainMenu.getGeneralSettings();
		if (gs.masterVolume == 1.0f)
			gs.masterVolume = 0.5f;
		app->getAudioEngine().setMasterVolume(gs.masterVolume);

		EngineSceneInfo sceneInfo;
		sceneInfo.pRenderComponentStorage = scene.ecs.getComponentStorage<RenderComponent>();
		sceneInfo.pTransformComponentStorage = scene.ecs.getComponentStorage<Transform>();
		sceneInfo.pActiveCamera = scene.activeCamera;
		sceneInfo.pLightComponentStorage = scene.ecs.getComponentStorage<LightSource>();
		sceneInfo.pPhysicsComponentStorage = scene.ecs.getComponentStorage<PhysicsComponent>();
		app->setActiveSceneInfo(sceneInfo);
		app->getWindow().setCursorLocked(false);

		playerEntity = scene.addEntity(); 

		Transform playerTransform;
		RenderComponent playerRender;
		PhysicsComponent playerPhysics;
		playerRender.setResourceLoader(app->getResourceloaderPtr());

		scene.entities[playerEntity].addComponent(playerTransform);
		scene.entities[playerEntity].selectComponent<Transform>().location = { 0.0f, -10000.0f, 0.0f }; // starts below to respawn and avoid collision wth blocks
		scene.entities[playerEntity].addComponent(playerRender);
		scene.entities[playerEntity].selectComponent<RenderComponent>().setMesh("resources/decodeSphere.obj");
		scene.entities[playerEntity].selectComponent<RenderComponent>().setMaterial("resources/metal.png");
		scene.entities[playerEntity].selectComponent<Transform>().scale = { 1.0f, 1.0f, 1.0f };
		scene.entities[playerEntity].selectComponent<RenderComponent>().setTransformIndex(scene.entities[playerEntity].getComponentIndex<Transform>());
		scene.entities[playerEntity].selectComponent<RenderComponent>().isVisible = false;
		scene.entities[playerEntity].addComponent<PhysicsComponent>(playerPhysics);
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().setTransformIndex(scene.entities[playerEntity].getComponentIndex<Transform>());
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().isStatic = false;
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().isFalling = true;
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().gravityScale = 0.01f;
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().collisionDimensions = { 1.0f, 4.0f, 1.0f };

		blockManager.setPlayerCollisionHalfSize({ 0.5f, 1.0f, 0.5f });

		scene.sceneCamera.farP = 100000;

		m_menuScene.init(app, scene, blockManager);

		blockManager.setEntityRemovalCallback([this](uint32_t removedIndex)
			{
				if (isFollowingBall && followedBallEntity == removedIndex)
					isFollowingBall = false;
				if (playerEntity > removedIndex) --playerEntity;
				m_menuScene.onEntityRemoved(removedIndex);
				if (hasRemotePlayer && remotePlayerEntity > removedIndex) --remotePlayerEntity;
				if (isFollowingBall && followedBallEntity > removedIndex) --followedBallEntity;
			});

		m_menuScene.create();

		// applies saved window settings on start
		pendingWindowChange = true;

		// applies saved vsync setting
		app->getWindow().setVSync(mainMenu.getGeneralSettings().vsync);
	}

	void onUpdate() override
	{
		float dT = app->getDeltaTIme() / 10.0f;
		lastDeltaTime = dT;

		if (pendingWindowChange)
		{
			pendingWindowChange = false;
			const auto& gs = mainMenu.getGeneralSettings();
			if (gs.isFullscreen != app->getWindow().isFullscreen())
				app->setWindowFullscreen(gs.isFullscreen);
			else if (!gs.isFullscreen)
			{
				app->resizeWindow(static_cast<unsigned int>(gs.windowWidth), static_cast<unsigned int>(gs.windowHeight));
				app->centerWindow();
			}
		}

		// animation of rotating block in menu
		if (!isGameStarted)
			m_menuScene.update(dT);

		// escape menu 
		const bool isEscPressed = inputHandler.keys.esc == true;
		const bool escAllowed = isGameStarted || isEscapeMenuOpen || CustomEngine::EngineGui::isMainMenuPanelOpen();
		if (isEscPressed && !wasEscPressed && !matchEnded && escAllowed)
		{
			isEscapeMenuOpen = !isEscapeMenuOpen;
			// revert changes which were not saved
			if (!isEscapeMenuOpen)
			{
				CustomEngine::EngineGui::revertEscapeMenuSettings(
					playerSpecs.mouseSensitivity,
					mainMenu.getGeneralSettings(),
					mainMenu.getControlsSettings());
				// visually apply reverted settings
				app->getWindow().setVSync(mainMenu.getGeneralSettings().vsync);
				app->getAudioEngine().setMasterVolume(mainMenu.getGeneralSettings().masterVolume);
				pendingWindowChange = true;
			}
			// lock cursor when opening escape menu from main menu
			app->getWindow().setCursorLocked(isActiveMatch() && !isEscapeMenuOpen);
			// reset mouse state to prevent sudden camera jumps
			if (!isEscapeMenuOpen)
				cameraController.resetMouseState();
		}
		wasEscPressed = isEscPressed;

		// while the escape menu is open, keep the network alive but skip game input
		if (isEscapeMenuOpen)
		{
			if (isGameStarted)
			{
				// freezing player when menu is opened
				PhysicsComponent& pausedPhysics = scene.entities[playerEntity].selectComponent<PhysicsComponent>();
				pausedPhysics.isFalling = false;
				pausedPhysics.locationInertia = { 0.0f, 0.0f, 0.0f };

				updateNetwork(dT);
			}
			return;
		}

		if (!isGameStarted)
			return;

		if (connectingToServer)
		{
			updateClientConnection(dT);
			return;
		}

		if (waitingForClient || pendingWorldInit)
		{
			if (waitingForClient && isHost && m_menuScene.isActive())
				m_menuScene.update(dT);
			updateNetwork(dT);
			return;
		}

		if (matchEnded)
		{
			updateNetwork(dT);
			return;
		}

		blockManager.ensureSpatialIndex();
		blockManager.updateSelection([this]() { app->getAudioEngine().playSound("button_click"); });

		float finalSpeed = playerSpecs.movementSpeed * dT / (100.0f * movementMultiplier);

		if (inputHandler.keys.lshift == true)
		{
			movementMultiplier = 1.5f;
		}
		else
		{
			movementMultiplier = 1.0f;
		}

		const float horizontalScale = flyMode ? flySpeedHorizontal : 1.0f;

		cameraController.update(
			-inputHandler.mouse.x,
			inputHandler.mouse.y,
			-((finalSpeed * horizontalScale * isControlKeyDown(mainMenu.getControlsSettings().keyBack)) - (finalSpeed * horizontalScale * isControlKeyDown(mainMenu.getControlsSettings().keyForward))), // forward
			(finalSpeed * horizontalScale * isControlKeyDown(mainMenu.getControlsSettings().keyRight)) - (finalSpeed * horizontalScale * isControlKeyDown(mainMenu.getControlsSettings().keyLeft)),   // right
			(flyMode) ? (float(isControlKeyDown(mainMenu.getControlsSettings().keyJump)) * flySpeedVertical * dT) - (float(inputHandler.keys.lctrl) * flySpeedVertical * dT) : 0.0f, // up
			true,
			scene.sceneCamera.rotation,
			scene.entities[playerEntity].selectComponent<Transform>().location,
			app->getWindowAspect(),
			playerSpecs.mouseSensitivity,
			dT,
			1.0f
		);

		const bool isVPressed = inputHandler.keys.v == true;
		if (isVPressed && !wasVPressed && mainMenu.getFlyMode())
		{
			flyMode = !flyMode;
		}
		wasVPressed = isVPressed;

		PhysicsComponent& playerPhysics = scene.entities[playerEntity].selectComponent<PhysicsComponent>();
		if (flyMode) // if cheaty Fly Mode is on
		{
			playerPhysics.gravityScale = 0.0f;
			playerPhysics.locationInertia = { 0.0f, 0.0f, 0.0f };
			playerPhysics.isFalling = false;
		}
		else // regular gameplay
		{
			playerPhysics.gravityScale = 0.01f;
			bool onGround = false;
			blockManager.resolveEntityBlockCollision(playerEntity, onGround);
			playerPhysics.isFalling = !onGround;

			const bool isSpacePressed = inputHandler.keys.space == true;

			// regens stamina on ground
			playerSpecs.tickStamina(isSpacePressed, onGround, dT);

			if (onGround)
			{
				app->getAudioEngine().stopSound("jetpack");
				if (isSpacePressed && !wasSpacePressed)
				{
					playerPhysics.locationInertia.y = playerSpecs.jumpStrength;
					app->getAudioEngine().playSound("jetpack");
				}
				wasSpacePressed = isSpacePressed;
			}
			else
			{
				const float thrust = playerSpecs.jetFly(inputHandler.keys.space, dT);
				if (thrust > 0.0f)
				{
					playerPhysics.locationInertia.y += thrust;
					if (playerPhysics.locationInertia.y > playerSpecs.maxJetSpeed)
						playerPhysics.locationInertia.y = playerSpecs.maxJetSpeed;
					if (!app->getAudioEngine().getSound("jetpack")->isPlaying())
						app->getAudioEngine().playSound("jetpack");
				}
				else
				{
					app->getAudioEngine().stopSound("jetpack");
					if (playerPhysics.locationInertia.y > 0.0f)
						playerPhysics.locationInertia.y -= playerSpecs.jetDecay * dT;
				}
				wasSpacePressed = false;
			}
		}

		// place camera at player model but 1 block up
		const Vec3& playerLoc = scene.entities[playerEntity].selectComponent<Transform>().location;
		scene.sceneCamera.location = { playerLoc.x, playerLoc.y + cameraHeightOffset, playerLoc.z };

		if (scene.entities[playerEntity].selectComponent<Transform>().location.y < -1000.0f)
		{
			teleportLocalPlayerToPlatform(blockManager.getPlayerTeamIndex());
		}

		if (hasRemotePlayer && scene.entities[remotePlayerEntity].selectComponent<Transform>().location.y < -1000.0f)
		{
			teleportRemotePlayerToPlatform(getOpposingPlatformIndex());
		}

		const bool isRPressed = inputHandler.keys.r == true;
		if (isRPressed && !wasRPressed)
		{
			isBuildMode = !isBuildMode;
		}
		wasRPressed = isRPressed;

		const bool isLPressed = inputHandler.mouse.lMouseButton == true;
		if (isLPressed && !wasLPressed)
		{
			if (inputHandler.keys.lshift == true)
			{
				Vec3 ballPosition{};
				Vec3 ballDirection{};
				uint8_t ballTeam = 0;
				if (blockManager.trySpawnBall(ballPosition, ballDirection, ballTeam))
				{
					app->getAudioEngine().playSound("ball_throw");
					if (isMultiplayer) sendBallSpawned(ballPosition, ballDirection, ballTeam);
				}
			}
			else
			{
				BlockManager::BlockHit hit{};
				if (blockManager.tryGetTargetedBlock(hit))
				{
					Vec3 center = blockManager.getBlockCenter(hit);

					if (isMultiplayer && !isHost)
					{
						sendBlockRemoved(center);
					}
					else
					{
						blockManager.removeBlockForPlayer(hit);
						app->getAudioEngine().playSound("block_remove");
						if (isMultiplayer) sendBlockRemoved(center);
					}
				}
			}
		}
		wasLPressed = isLPressed;

		const bool isRightMousePressed = inputHandler.mouse.rMouseButton == true;
		if (isRightMousePressed && !wasRightMousePressed)
		{
			Vec3 placement{};
			size_t blockType = 0;
			if (blockManager.trySpawnBlock(placement, blockType))
			{
				app->getAudioEngine().playSound("block_place");
				if (isMultiplayer) sendBlockPlaced(placement, blockType);
			}
		}
		wasRightMousePressed = isRightMousePressed;

		const bool isCPressed = inputHandler.keys.c == true;
		if (isCPressed && !wasCPressed)
		{
			if (!isFollowingBall)
			{
				if (blockManager.hasActiveBalls())
				{
					followedBallEntity = blockManager.getLastBallEntity();
					isFollowingBall = true;
				}
			}
			else
			{
				isFollowingBall = false;
			}
		}
		wasCPressed = isCPressed;

		if (isFollowingBall)
		{
			if (blockManager.isBallEntityAlive(followedBallEntity))
			{
				scene.sceneCamera.location = scene.entities[followedBallEntity].selectComponent<Transform>().location;
			}
			else
			{
				isFollowingBall = false;
				scene.sceneCamera.location = { playerLoc.x, playerLoc.y + cameraHeightOffset, playerLoc.z };
			}
		}

		const size_t ballCountBefore = blockManager.getBallCount();
		blockManager.handleBallBlockCollisions();
		if (blockManager.getBallCount() < ballCountBefore)
			app->getAudioEngine().playSound("ball_hit");
		blockManager.pruneFallenBalls(-1000.0f);

		// timer and match conditions
		{
			const size_t playerPlatform = blockManager.getPlayerTeamIndex();
			const size_t enemyPlatform = (playerPlatform == 0) ? 1 : 0;
			const int playerBlocks = static_cast<int>(blockManager.getPlatformBlockCount(playerPlatform));
			const int enemyBlocks = static_cast<int>(blockManager.getPlatformBlockCount(enemyPlatform));

			const bool enemyLost = (enemyBlocks <= 0) || (minBlocksForLoss > 0 && enemyBlocks < minBlocksForLoss);
			const bool playerLost = (playerBlocks <= 0) || (minBlocksForLoss > 0 && playerBlocks < minBlocksForLoss);

			matchTimer -= dT / 6000.0f;

			if (enemyLost && !playerLost)
				endMatch(true);
			else if (playerLost && !enemyLost)
				endMatch(false);
			else if (enemyLost && playerLost)
				endMatch(playerBlocks >= enemyBlocks);
			else if (matchTimer <= 0.0f)
				endMatch(playerBlocks >= enemyBlocks);
		}

		updateNetwork(dT);
	}

	void onImGuiRender() override
	{
		const bool inActiveGameplay = isActiveMatch() && !isEscapeMenuOpen && !matchEnded;
		CustomEngine::EngineGui::showGameVersion(GAME_VERSION_STRING, !inActiveGameplay, [this]() { app->getAudioEngine().playSound("button_click"); });

		if (!isGameStarted)
		{
			bool settingsClicked = false;
			bool quitClicked     = false;
			if (!isEscapeMenuOpen)
			{
				if (mainMenu.render(settingsClicked, quitClicked, connectionError.c_str(), [this]() { app->getAudioEngine().playSound("button_click"); }))
				{
					startGame();
				}
				if (settingsClicked)
				{
					isEscapeMenuOpen = true;
					mainMenu.getGeneralSettings().settingsOpen = true;
				}
				if (quitClicked)
					app->terminate();
			}
		}
		else if (waitingForClient)
		{
			if (CustomEngine::EngineGui::showWaitingForPlayers([this]() { app->getAudioEngine().playSound("button_click"); }))
				returnToMenu("");
		}
		else if (connectingToServer)
		{
			CustomEngine::EngineGui::showOverlayMessage("Connecting to server...");
		}
		else if (pendingWorldInit)
		{
			CustomEngine::EngineGui::showOverlayMessage("Loading world...");
		}
		else if (!matchEnded)
		{
			const size_t blockTypeCount = blockManager.getBlockTypeCount();
			std::vector<const char*> materials(blockTypeCount);
			std::vector<int> counts(blockTypeCount);

			for (size_t i = 0; i < blockTypeCount; ++i)
			{
				materials[i] = blockManager.getBlockTypeMaterial(i);
				counts[i] = playerSpecs.inventory.getCount(blockManager.getBlockTypeId(i));
			}

			const size_t playerPlatform = blockManager.getPlatformIndexForPosition(scene.sceneCamera.location);
			const size_t enemyPlatform = (playerPlatform == 0) ? 1 : 0;
			const int playerPoints = static_cast<int>(blockManager.getPlatformBlockCount(playerPlatform));
			const int enemyPoints = static_cast<int>(blockManager.getPlatformBlockCount(enemyPlatform));
			CustomEngine::EngineGui::showTeamScoreBar(playerPoints, enemyPoints);

			CustomEngine::EngineGui::showMatchTimer(matchTimer);

			CustomEngine::EngineGui::showStamina(playerSpecs.stamina, playerSpecs.maxStamina);
			CustomEngine::EngineGui::showInventoryBar(materials, counts, blockManager.getSelectedBlockType());
		}

		// FPS overlay and crosshair
		{
			const auto& gs = mainMenu.getGeneralSettings();
			if (gs.showFps)
			{
				const float fps = lastDeltaTime > 0.0f ? (1000.0f / (lastDeltaTime * 10.0f)) : 0.0f;
				CustomEngine::EngineGui::showFpsOverlay(fps, gs.fpsCorner);
			}
			if (inActiveGameplay)
				CustomEngine::EngineGui::showCrosshair(gs);
		}

		// escape menu
		if (isEscapeMenuOpen && !matchEnded)
		{
			const bool needsSettings = waitingForClient || pendingWorldInit || connectingToServer
				|| CustomEngine::EngineGui::isMainMenuPanelOpen();
			mainMenu.getGeneralSettings().isFullscreen = app->getWindow().isFullscreen();
			const auto escResult = CustomEngine::EngineGui::showEscapeMenu(isActiveMatch(), playerSpecs.mouseSensitivity, mainMenu.getGeneralSettings(), mainMenu.getControlsSettings(),
				[this]() { app->getAudioEngine().playSound("button_click"); }, needsSettings);
			if (escResult.resizeRequested)
			{
				app->setRenderResolution(0, 0);
				pendingWindowChange = true;
			}
			if (escResult.renderResolutionChanged)
			{
				app->setRenderResolution(
					static_cast<unsigned int>(escResult.renderWidth),
					static_cast<unsigned int>(escResult.renderHeight));
			}
			if (escResult.saveRequested)
			{
				mainMenu.saveSettings();
				if (!isActiveMatch())
				{
					isEscapeMenuOpen = false;
					mainMenu.getGeneralSettings().settingsOpen = false;
				}
			}
			if (escResult.vsyncChanged)
			{
				app->getWindow().setVSync(mainMenu.getGeneralSettings().vsync);
			}
			if (escResult.volumeChanged)
			{
				app->getAudioEngine().setMasterVolume(mainMenu.getGeneralSettings().masterVolume);
			}
			if (escResult.playContinueClicked)
				{
					isEscapeMenuOpen = false;
					mainMenu.getGeneralSettings().settingsOpen = false;
					app->getWindow().setCursorLocked(isActiveMatch());
				}
				if (escResult.disconnectClicked)
				{
					mainMenu.getGeneralSettings().settingsOpen = false;
					returnToMenu("");
				}
				if (escResult.exitClicked)
				{
					mainMenu.getGeneralSettings().settingsOpen = false;
					app->terminate();
				}
		}

		// match result
		if (matchEnded)
		{
			if (CustomEngine::EngineGui::showMatchResult(localPlayerWon, [this]() { app->getAudioEngine().playSound("button_click"); }))
				returnToMenu("");
		}
	}

private:
	// network message sending functions
	bool isActiveMatch() const
	{
		return isGameStarted && !connectingToServer && !waitingForClient && !pendingWorldInit;
	}

	bool isControlKeyDown(int imguiKey) const
	{
		return CustomEngine::EngineGui::isKeyDown(imguiKey);
	}

	size_t getOpposingPlatformIndex() const
	{
		return (blockManager.getPlayerTeamIndex() == 0) ? 1 : 0;
	}

	void teleportLocalPlayerToPlatform(size_t platformIndex)
	{
		Transform& playerTransform = scene.entities[playerEntity].selectComponent<Transform>();
		PhysicsComponent& playerPhysics = scene.entities[playerEntity].selectComponent<PhysicsComponent>();
		const float playerHalfHeight = playerPhysics.collisionDimensions.y * 0.5f * playerTransform.scale.y;
		playerTransform.location = blockManager.getPlatformSpawnLocation(platformIndex, playerHalfHeight);
		playerPhysics.locationInertia = { 0.0f, 0.0f, 0.0f };
	}

	void teleportRemotePlayerToPlatform(size_t platformIndex)
	{
		if (!hasRemotePlayer)
		{
			return;
		}

		Transform& remoteTransform = scene.entities[remotePlayerEntity].selectComponent<Transform>();
		const float remoteHalfHeight = 1.0f;
		remoteTransform.location = blockManager.getPlatformSpawnLocation(platformIndex, remoteHalfHeight);
	}

	void createRemotePlayerEntity()
	{
		if (hasRemotePlayer)
		{
			return;
		}

		remotePlayerEntity = scene.addEntity();

		Transform remoteTransform;
		RenderComponent remoteRender;
		remoteRender.setResourceLoader(app->getResourceloaderPtr());

		scene.entities[remotePlayerEntity].addComponent(remoteTransform);
		scene.entities[remotePlayerEntity].selectComponent<Transform>().location = { 0.0f, 0.0f, 0.0f };
		scene.entities[remotePlayerEntity].addComponent(remoteRender);
		scene.entities[remotePlayerEntity].selectComponent<RenderComponent>().setMesh("resources/decodeSphere.obj");
		scene.entities[remotePlayerEntity].selectComponent<RenderComponent>().setMaterial("resources/Stone_05-256x256.png");
		scene.entities[remotePlayerEntity].selectComponent<RenderComponent>().setTransformIndex(scene.entities[remotePlayerEntity].getComponentIndex<Transform>());
		scene.entities[remotePlayerEntity].selectComponent<Transform>().scale = { 1.0f, 1.0f, 1.0f };

		hasRemotePlayer = true;
		teleportRemotePlayerToPlatform(getOpposingPlatformIndex());
	}

	void applyRemoteState(const NetMessagePacket& packet)
	{
		if (!hasRemotePlayer || packet.messageType != static_cast<uint32_t>(NetMessageType::PlayerState))
		{
			return;
		}

		Transform& remoteTransform = scene.entities[remotePlayerEntity].selectComponent<Transform>();
		remoteTransform.location = { packet.px, packet.py, packet.pz };
		remoteTransform.rotation = { packet.rw, packet.rx, packet.ry, packet.rz };
	}

	void sendLocalState()
	{
		Transform& localTransform = scene.entities[playerEntity].selectComponent<Transform>();

		NetMessagePacket packet{};
		packet.messageType = static_cast<uint32_t>(NetMessageType::PlayerState);
		packet.px = localTransform.location.x;
		packet.py = localTransform.location.y;
		packet.pz = localTransform.location.z;
		packet.tmr = 0.0f;
		packet.rw = scene.sceneCamera.rotation.w;
		packet.rx = scene.sceneCamera.rotation.x;
		packet.ry = scene.sceneCamera.rotation.y;
		packet.rz = scene.sceneCamera.rotation.z;

		m_net.pushOutgoing(packet);
	}

	void updateClientConnection(float /*dT*/)
	{
		const float elapsed = std::chrono::duration<float>(
			std::chrono::steady_clock::now() - connectStartTime).count();

		// if connection fails or we time out, return to menu with an error
		if (m_net.hasConnectionFailed() || elapsed >= maxConnectTimeoutSeconds)
		{
			returnToMenu("Failed to connect: server not found");
			return;
		}

		if (m_net.isConnected())
		{
			connectingToServer = false;
		}
	}

	void endMatch(bool won)
	{
		matchEnded = true;
		localPlayerWon = won;
		app->getWindow().setCursorLocked(false);
	}

	void returnToMenu(const std::string& error)
	{
		isGameStarted = false;
		isMultiplayer = false;
		isHost = false;
		connectingToServer = false;
		waitingForClient = false;
		pendingWorldInit = false;
		worldSeedSent = false;
		versionHandshakeSent = false;
		versionConfirmed = false;
		versionRejectTimer = -1.0f;
		reconnectTimer = 0.0f;
			connectStartTime = {};
			networkSendAccumulator = 0.0f;
		isEscapeMenuOpen = false;
		pendingWindowChange = false;
		matchTimer = 0.0f;
		matchEnded = false;
		gamePausedForReconnect = false;
		m_net.stop();
		connectionError = error;
		mainMenu.resetGameSettings();
		CustomEngine::EngineGui::resetMainMenuState();
		app->getWindow().setCursorLocked(false);

		// clear entities and block so they not overlap the animation
		m_menuScene.destroy();
		cleanupGameEntities();

		// restart animation in menu
		m_menuScene.create();
	}

	bool gamePausedForReconnect = false;

	void updateNetwork(float dT)
	{
		if (!isMultiplayer || !hasRemotePlayer)
		{
			return;
		}

		// disconnect detection
		if (m_net.hasRemoteDisconnected())
		{
			if (isHost)
				{
				// client left: wait for another client to join
					gamePausedForReconnect = true;
						worldSeedSent        = false;
						versionConfirmed     = false;
						versionHandshakeSent = false;
						waitingForClient     = true;
						app->getWindow().setCursorLocked(false);
						blockManager.clearWorld();
						scene.entities[playerEntity].selectComponent<Transform>().location = { 0.0f, -10000.0f, 0.0f };
						scene.entities[playerEntity].selectComponent<PhysicsComponent>().locationInertia = { 0.0f, 0.0f, 0.0f };
						m_menuScene.create();
					return;
				}
			else
			{
				// host left: kick the client back to the main menu
				returnToMenu("Host has left the game.");
				return;
			}
		}

		if (pendingVersionReject)
		{
			// kick the mismatched client by restarting the server, then resume waiting.
			pendingVersionReject = false;
			m_net.stop();
			worldSeedSent = false;
			versionConfirmed = false;
			waitingForClient = true;
			m_net.startServer(serverPort);
			return;
		}

		networkSendAccumulator += dT;
		const bool shouldSend = networkSendAccumulator >= (1.0f / 30.0f);
		if (shouldSend)
		{
			networkSendAccumulator = 0.0f;
		}

		NetMessagePacket packet{};

		if (isHost)
		{
			while (m_net.popIncoming(packet))
			{
				if (packet.messageType == static_cast<uint32_t>(NetMessageType::VersionHandshake))
				{
					if (packet.seed == GameVersion::VERSION_HASH)
					{
						versionConfirmed = true;
					}
					else
					{
						NetMessagePacket rejectPacket{};
						rejectPacket.messageType = static_cast<uint32_t>(NetMessageType::VersionHandshake);
						rejectPacket.seed = GameVersion::VERSION_HASH;
						m_net.pushOutgoing(rejectPacket);
						pendingVersionReject = true;
						return;
					}
				}
				else if (packet.messageType == static_cast<uint32_t>(NetMessageType::PlayerState))
				{
					applyRemoteState(packet);
				}
				else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BlockRemoveRequest))
				{
					Vec3 center{ packet.bx, packet.by, packet.bz };
					blockManager.removeBlockAtPositionRemote(center);

					NetMessagePacket removedPacket{};
					removedPacket.messageType = static_cast<uint32_t>(NetMessageType::BlockRemoved);
					removedPacket.bx = packet.bx;
					removedPacket.by = packet.by;
					removedPacket.bz = packet.bz;
					m_net.pushOutgoing(removedPacket);
				}
				else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BlockPlaced))
				{
					Vec3 placement{ packet.bx, packet.by, packet.bz };
					blockManager.spawnRemoteBlock(static_cast<size_t>(packet.seed), placement);
				}
				else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BallSpawned))
				{
					Vec3 position{ packet.px, packet.py, packet.pz };
					Vec3 direction{ packet.bx, packet.by, packet.bz };
					blockManager.spawnRemoteBall(position, direction, static_cast<uint8_t>(packet.seed));
				}
			}

			if (m_net.isClientPresent() && !worldSeedSent && versionConfirmed)
			{
				initializeWorld();

				// send all game settings to client
				NetMessagePacket settingsPacket{};
				settingsPacket.messageType = static_cast<uint32_t>(NetMessageType::GameSettings);
				settingsPacket.px  = mainMenu.getPlayerSettings().moveSpeed;
				settingsPacket.py  = mainMenu.getPlayerSettings().flySpeedHorizontal;
				settingsPacket.pz  = mainMenu.getPlayerSettings().flySpeedVertical;
				settingsPacket.tmr = mainMenu.getPlayerSettings().jetMaxSpeed;
				settingsPacket.rw  = mainMenu.getPlayerSettings().ballSpeed;
				settingsPacket.rx  = static_cast<float>(mainMenu.getWorldSettings().maxBuildHeightBlocks);
				settingsPacket.ry  = static_cast<float>(mainMenu.getWorldSettings().platformBaseHeight);
				settingsPacket.rz  = static_cast<float>(mainMenu.getMatchSettings().matchDurationMinutes);
				settingsPacket.bx  = static_cast<float>(mainMenu.getMatchSettings().minBlocksForLoss);
				settingsPacket.by  = static_cast<float>(mainMenu.getMatchSettings().initialBlockCount);
				m_net.pushOutgoing(settingsPacket);

				NetMessagePacket seedPacket{};
				seedPacket.messageType = static_cast<uint32_t>(NetMessageType::WorldSeed);
				seedPacket.seed = worldSeed;
				seedPacket.px = static_cast<float>(mainMenu.getPlatformSize());
				seedPacket.py = static_cast<float>(mainMenu.getPlatformOffsetBlocks());
				seedPacket.pz = flyMode ? 1.0f : 0.0f;
				seedPacket.tmr = 0.0f;
				m_net.pushOutgoing(seedPacket);
				worldSeedSent = true;
				waitingForClient = false;
				gamePausedForReconnect = false;
				m_menuScene.destroy();
				app->getWindow().setCursorLocked(true);
			}

			if (m_net.isClientPresent() && shouldSend)
			{
				sendLocalState();
			}

			return;
		}

		if (!m_net.isConnected())
		{
			if (m_net.isConnecting()) return;

			reconnectTimer += dT;
			if (reconnectTimer >= 1.0f)
			{
				reconnectTimer = 0.0f;
				versionHandshakeSent = false;
				m_net.startClient(serverAddress, serverPort);
			}
			return;
		}

		if (!versionHandshakeSent)
		{
			NetMessagePacket versionPacket{};
			versionPacket.messageType = static_cast<uint32_t>(NetMessageType::VersionHandshake);
			versionPacket.seed = GameVersion::VERSION_HASH;
			m_net.pushOutgoing(versionPacket);
			versionHandshakeSent = true;
		}

		while (m_net.popIncoming(packet))
		{
			if (packet.messageType == static_cast<uint32_t>(NetMessageType::VersionHandshake))
			{
				returnToMenu("Version mismatch: host is running a different game version.");
				return;
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::PlayerState))
			{
				applyRemoteState(packet);
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::GameSettings))
			{
				moveSpeed          = packet.px;
				flySpeedHorizontal = packet.py;
				flySpeedVertical   = packet.pz;
				jetMaxSpeed        = packet.tmr;
				ballSpeed          = packet.rw;
				playerSpecs.movementSpeed = moveSpeed;
				playerSpecs.maxJetSpeed   = jetMaxSpeed;
				blockManager.setBallSpeed(ballSpeed);
				blockManager.setMaxBuildHeight(static_cast<int>(packet.rx));
				blockManager.setPlatformBaseHeight(static_cast<int>(packet.ry));
				blockManager.setInitialBlockCount(static_cast<int>(packet.by));
				matchDuration    = packet.rz;
				matchTimer       = matchDuration;
				minBlocksForLoss = static_cast<int>(packet.bx);
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::WorldSeed))
			{
				if (pendingWorldInit)
				{
					blockManager.setWorldSeed(packet.seed);
					blockManager.setPlatformSettings(static_cast<size_t>(packet.px), static_cast<int>(packet.py));
					flyMode = packet.pz != 0.0f;
					initializeWorld();
					pendingWorldInit = false;
					app->getWindow().setCursorLocked(true);
				}
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BlockRemoved))
			{
				Vec3 center{ packet.bx, packet.by, packet.bz };
				blockManager.removeBlockAtPositionRemote(center);
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BlockPlaced))
			{
				Vec3 placement{ packet.bx, packet.by, packet.bz };
				blockManager.spawnRemoteBlock(static_cast<size_t>(packet.seed), placement);
			}
			else if (packet.messageType == static_cast<uint32_t>(NetMessageType::BallSpawned))
			{
				Vec3 position{ packet.px, packet.py, packet.pz };
				Vec3 direction{ packet.bx, packet.by, packet.bz };
				blockManager.spawnRemoteBall(position, direction, static_cast<uint8_t>(packet.seed));
			}
		}

		if (shouldSend)
		{
			sendLocalState();
		}
	}

	void startGame()
	{
		if (isGameStarted)
			return;

		connectionError.clear();
		isGameStarted = true;

		{
			const auto& ps = mainMenu.getPlayerSettings();
			moveSpeed          = ps.moveSpeed;
			flySpeedHorizontal = ps.flySpeedHorizontal;
			flySpeedVertical   = ps.flySpeedVertical;
			jetMaxSpeed        = ps.jetMaxSpeed;
			ballSpeed          = ps.ballSpeed;

			playerSpecs.movementSpeed = moveSpeed;
			playerSpecs.maxJetSpeed   = jetMaxSpeed;
			blockManager.setBallSpeed(ballSpeed);
		}

		{
			const auto& ms = mainMenu.getMatchSettings();
			matchDuration    = static_cast<float>(ms.matchDurationMinutes);
			minBlocksForLoss = ms.minBlocksForLoss;
			matchTimer       = matchDuration;
			matchEnded       = false;
			localPlayerWon   = false;
		}

		isMultiplayer = mainMenu.isMultiplayerEnabled();
		if (isMultiplayer)
		{
			isHost = mainMenu.isHost();
			serverAddress = mainMenu.getServerAddress();
			serverPort = static_cast<uint16_t>(mainMenu.getServerPort());

			createRemotePlayerEntity();

			if (isHost)
			{
				flyMode = mainMenu.getFlyMode();
				blockManager.setPlatformSettings(static_cast<size_t>(mainMenu.getPlatformSize()), mainMenu.getPlatformOffsetBlocks());
				blockManager.setMaxBuildHeight(mainMenu.getWorldSettings().maxBuildHeightBlocks);
				blockManager.setPlatformBaseHeight(mainMenu.getWorldSettings().platformBaseHeight);
				blockManager.setInitialBlockCount(mainMenu.getMatchSettings().initialBlockCount);
				worldSeed = static_cast<uint32_t>(std::random_device{}());
				blockManager.setWorldSeed(worldSeed);
				m_net.startServer(serverPort);
				waitingForClient = true;
				m_menuScene.create();
			}
			else
			{
				pendingWorldInit = true;
				connectingToServer = true;
				connectStartTime = std::chrono::steady_clock::now();
				m_net.startClient(serverAddress, serverPort);
			}
		}
		else
		{
			flyMode = mainMenu.getFlyMode();
			blockManager.setPlatformSettings(static_cast<size_t>(mainMenu.getPlatformSize()), mainMenu.getPlatformOffsetBlocks());
			blockManager.setMaxBuildHeight(mainMenu.getWorldSettings().maxBuildHeightBlocks);
			blockManager.setPlatformBaseHeight(mainMenu.getWorldSettings().platformBaseHeight);
			blockManager.setInitialBlockCount(mainMenu.getMatchSettings().initialBlockCount);
			m_menuScene.destroy();
			initializeWorld();
		}
		app->getWindow().setCursorLocked(!waitingForClient && !connectingToServer);
	}

	void initializeWorld()
	{
		blockManager.initializeInventory();
		blockManager.createInitialPlatform();

		const size_t localPlatformIndex = (isMultiplayer && !isHost) ? 1 : 0;
		teleportLocalPlayerToPlatform(localPlatformIndex);
		blockManager.setPlayerTeamIndex(localPlatformIndex);

		if (hasRemotePlayer)
		{
			teleportRemotePlayerToPlatform(getOpposingPlatformIndex());
		}
	}

	void sendBlockRemoved(const Vec3& center)
	{
		NetMessagePacket packet{};
		packet.messageType = static_cast<uint32_t>(NetMessageType::BlockRemoved);
		packet.bx = center.x;
		packet.by = center.y;
		packet.bz = center.z;

		if (isHost)
		{
			if (m_net.isClientPresent())
				m_net.pushOutgoing(packet);
		}
		else
		{
			if (m_net.isConnected())
			{
				packet.messageType = static_cast<uint32_t>(NetMessageType::BlockRemoveRequest);
				m_net.pushOutgoing(packet);
			}
		}
	}

	void sendBlockPlaced(const Vec3& placement, size_t blockType)
	{
		NetMessagePacket packet{};
		packet.messageType = static_cast<uint32_t>(NetMessageType::BlockPlaced);
		packet.bx = placement.x;
		packet.by = placement.y;
		packet.bz = placement.z;
		packet.seed = static_cast<uint32_t>(blockType);

		if (isHost)
		{
			if (m_net.isClientPresent())
				m_net.pushOutgoing(packet);
		}
		else
		{
			if (m_net.isConnected())
				m_net.pushOutgoing(packet);
		}
	}

	void sendBallSpawned(const Vec3& position, const Vec3& direction, uint8_t teamIndex)
	{
		NetMessagePacket packet{};
		packet.messageType = static_cast<uint32_t>(NetMessageType::BallSpawned);
		packet.px = position.x;
		packet.py = position.y;
		packet.pz = position.z;
		packet.bx = direction.x;
		packet.by = direction.y;
		packet.bz = direction.z;
		packet.seed = static_cast<uint32_t>(teamIndex);

		if (isHost)
		{
			if (m_net.isClientPresent())
				m_net.pushOutgoing(packet);
		}
		else
		{
			if (m_net.isConnected())
				m_net.pushOutgoing(packet);
		}
	}

	void cleanupGameEntities()
	{
		// Clear all blocks, balls, and platforms from BlockManager
		blockManager.clearWorld();

		// Hide the player entity by moving it far away
		scene.entities[playerEntity].selectComponent<Transform>().location = { 0.0f, -10000.0f, 0.0f };
		scene.entities[playerEntity].selectComponent<PhysicsComponent>().locationInertia = { 0.0f, 0.0f, 0.0f };

		// Remove remote player entity if it exists
		if (hasRemotePlayer)
		{
			uint32_t remoteIndex = remotePlayerEntity;
			scene.removeEntity(remoteIndex);
			
			// Update indices of entities created after the remote player
			blockManager.updateEntityIndicesAfterRemoval(remoteIndex);
			if (playerEntity > remoteIndex) --playerEntity;
			m_menuScene.onEntityRemoved(remoteIndex);
			
			hasRemotePlayer = false;
			remotePlayerEntity = 0;
		}

		// Reset ball following state
		isFollowingBall = false;
		followedBallEntity = 0;
	}
};