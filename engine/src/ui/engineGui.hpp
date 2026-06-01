#pragma once

#include "../engineCore/core.h"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace CustomEngine
{
	namespace EngineGui
	{
		struct SavedServerView
		{
			const char* name = "";
			const char* address = "";
			int port = 28000;
		};

		struct ServerListResult
		{
			int selectIndex = -1;
			int removeIndex = -1;
			bool saveRequested = false;
		};

		struct EscapeMenuResult
		{
			bool playContinueClicked     = false;
			bool exitClicked             = false;
			bool disconnectClicked       = false;
			bool resizeRequested         = false;
			bool renderResolutionChanged = false;
			bool saveRequested           = false;
			bool vsyncChanged            = false;
			bool volumeChanged           = false;
			bool controlsChanged         = false;
			int  renderWidth             = 0;
			int  renderHeight            = 0;
		};

		struct PlayerSettings
		{
			float moveSpeed          = 5.0f;
			float flySpeedHorizontal = 1.0f;
			float flySpeedVertical   = 0.05f;
			float jumpHeight         = 0.8f;
			float jetStrength        = 0.005f;
			float jetMaxSpeed        = 0.3f;
			float ballSpeed          = 3.0f;
		};

		struct WorldSettings
		{
			int maxBuildHeightBlocks = 8;
			int platformBaseHeight   = 5;
		};

		struct MatchSettings
		{
			int matchDurationMinutes = 5;
			int minBlocksForLoss     = 10;
			int initialBlockCount    = 10;
		};

		struct SettingsSlotResult
		{
			int saveSlot = -1;
			int loadSlot = -1;
		};

		enum class FpsCorner { TopLeft = 0, TopRight, BottomLeft, BottomRight };

		struct GeneralSettings
		{
			int       windowWidth    = 1280;
			int       windowHeight   = 720;
			bool      isFullscreen   = false;
			bool      vsync          = true;
			bool      showFps        = false;
			FpsCorner fpsCorner      = FpsCorner::TopLeft;
			bool      settingsOpen   = false;
			float     masterVolume   = 1.0f;
			bool      showCrosshair  = true;
				float     crosshairSize  = 10.0f;
				float     crosshairThickness = 1.5f;
				float     crosshairColor[4]  = { 1.0f, 1.0f, 1.0f, 0.9f };
				float     cloudDrawDistance  = 9500.0f;
			};

		struct ControlsSettings
		{
			int keyForward   = 568;  // ImGuiKey_W
			int keyBack      = 564;  // ImGuiKey_S
			int keyLeft      = 546;  // ImGuiKey_A
			int keyRight     = 549;  // ImGuiKey_D
			int keyJump      = 524;  // ImGuiKey_Space
			int keyFlyToggle = 567;  // ImGuiKey_V
			int keyBuildMode = 563;  // ImGuiKey_R
			int keyCamFollow = 548;  // ImGuiKey_C
		};

		CUSTOMENGINE_API bool isKeyDown(int imguiKey);
		CUSTOMENGINE_API void showFlyMode(bool flyMode);
		CUSTOMENGINE_API void showBlockCount(int blockCount);
		CUSTOMENGINE_API void showBuildingMode(bool buildingMode);
		CUSTOMENGINE_API void showStamina(float stamina, float maxStamina);
		CUSTOMENGINE_API void showInventoryBar(const std::vector<const char*>& materials, const std::vector<int>& counts, size_t selectedIndex);
		CUSTOMENGINE_API void showPoints(int points);
		CUSTOMENGINE_API void showTeamScoreBar(int playerPoints, int enemyPoints);
		CUSTOMENGINE_API void showFpsOverlay(float fps, FpsCorner corner);
		CUSTOMENGINE_API void showCrosshair(const GeneralSettings& generalSettings);
		CUSTOMENGINE_API void showOverlayMessage(const char* message);
		CUSTOMENGINE_API bool showWaitingForPlayers(const std::function<void()>& onButtonClick = nullptr);
		CUSTOMENGINE_API void showGameVersion(const char* version, bool changelogAllowed = true, const std::function<void()>& onButtonClick = nullptr);
		CUSTOMENGINE_API void resetMainMenuState();
		CUSTOMENGINE_API bool isMainMenuPanelOpen();
		CUSTOMENGINE_API bool showMainMenu(bool& flyMode, int& platformOffsetBlocks, int& platformSize, bool& multiplayerEnabled, int& networkRole, char* serverAddress, size_t serverAddressSize, int& serverPort, const std::vector<SavedServerView>& savedServers, ServerListResult& serverListResult, char* newServerName, size_t newServerNameSize, const char* errorMessage, PlayerSettings& playerSettings, WorldSettings& worldSettings, MatchSettings& matchSettings, const bool slotExists[3], SettingsSlotResult& slotResult, bool& outSettingsClicked, bool& outQuitClicked, const std::function<void()>& onButtonClick = nullptr);
		CUSTOMENGINE_API void showMatchTimer(float remainingMinutes);
		CUSTOMENGINE_API bool showMatchResult(bool playerWon, const std::function<void()>& onButtonClick = nullptr);
		CUSTOMENGINE_API EscapeMenuResult showEscapeMenu(bool isInGame, float& mouseSensitivity, GeneralSettings& generalSettings, ControlsSettings& controlsSettings, const std::function<void()>& onButtonClick = nullptr, bool allowSettings = false);
		// Call this when the escape menu is closed externally (e.g. ESC key) so unsaved settings are reverted
		CUSTOMENGINE_API void revertEscapeMenuSettings(float& mouseSensitivity, GeneralSettings& generalSettings, ControlsSettings& controlsSettings);
	}
}