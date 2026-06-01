#include "engineGui.hpp"
#include "changelog.hpp"
#include "imgui.h"
#include "../renderer/helper/OGLImport.hpp"
#include "../resourceLoader/materialData.hpp"
#include "../resourceLoader/imgLoadFunction.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <string>
#include <unordered_map>

namespace
{
	struct InventoryTextureCache
	{
		std::unordered_map<std::string, unsigned int>                textures;
		std::unordered_map<std::string, std::future<MaterialData>>   pendingLoads;

		unsigned int getTextureId(const char* filePath)
		{
			if (!filePath || filePath[0] == '\0')
			{
				return 0;
			}

			auto it = textures.find(filePath);
			if (it != textures.end())
			{
				return it->second;
			}

			auto pit = pendingLoads.find(filePath);
			if (pit != pendingLoads.end())
			{
				if (pit->second.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				{
					return 0; 
				}

				MaterialData material = pit->second.get();
				pendingLoads.erase(pit);

				if (material.textureData.empty())
				{
					return 0;
				}

				unsigned int textureId = 0;
				checkOGL(glGenTextures(1, &textureId));
				checkOGL(glBindTexture(GL_TEXTURE_2D, textureId));
				checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				checkOGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				checkOGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, material.textureWidth, material.textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, material.textureData.data()));
				checkOGL(glBindTexture(GL_TEXTURE_2D, 0));

				textures.emplace(filePath, textureId);
				return textureId;
			}

			std::string path(filePath);
			pendingLoads.emplace(filePath, std::async(std::launch::async, [path]()
			{
				MaterialData mat{};
				loadPngToMaterialData(path, mat);
				return mat;
			}));

			return 0;
		}
	};

	InventoryTextureCache& getInventoryTextureCache() 
	{
		static InventoryTextureCache cache;
		return cache;
	}
}

namespace CustomEngine::EngineGui
{
	bool isKeyDown(int imguiKey)
	{
		return ImGui::IsKeyDown(static_cast<ImGuiKey>(imguiKey));
	}

	void showFlyMode(bool flyMode)
	{
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("FlyModeOverlay", nullptr, flags))
		{
			ImGui::TextUnformatted(flyMode ? "Fly mode: ON" : "Fly mode: OFF");
		}
		ImGui::End();
	}

	void showBlockCount(int blockCount)
	{
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::SetNextWindowPos(ImVec2(12.0f, 40.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("BlockCountOverlay", nullptr, flags))
		{
			ImGui::Text("Blocks: %d", blockCount);
		}
		ImGui::End();
	}

	void showBuildingMode(bool buildingMode)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const ImVec2 bottomRight =
			ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - margin,
				viewport->WorkPos.y + viewport->WorkSize.y - margin);

		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::SetNextWindowPos(bottomRight, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("BuildingModeOverlay", nullptr, flags))
		{
			ImGui::TextUnformatted(buildingMode ? "Mode: Building" : "Mode: Destruction");
		}
		ImGui::End();
	}

	void showStamina(float stamina, float maxStamina)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const float inventoryOffset = 64.0f;
		const ImVec2 bottomCenter =
			ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
				viewport->WorkPos.y + viewport->WorkSize.y - margin - inventoryOffset);

		const float fraction = (maxStamina > 0.0f) ? std::clamp(stamina / maxStamina, 0.0f, 1.0f) : 0.0f;
		char overlay[32];
		std::snprintf(overlay, sizeof(overlay), "%.0f%%", fraction * 100.0f);

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(bottomCenter, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("StaminaOverlay", nullptr, flags | ImGuiWindowFlags_NoBackground))
		{
			ImGui::ProgressBar(fraction, ImVec2(160.0f, 0.0f), overlay);
		}
		ImGui::End();
	}

	void showInventoryBar(const std::vector<const char*>& materials, const std::vector<int>& counts, size_t selectedIndex)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const ImVec2 bottomCenter =
			ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
				viewport->WorkPos.y + viewport->WorkSize.y - margin);

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(bottomCenter, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("InventoryOverlay", nullptr, flags))
		{
			const ImVec2 cellSize(48.0f, 48.0f);
			const ImVec2 imagePadding(6.0f, 6.0f);
			const ImU32 cellFill = ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.08f, 0.55f));
			const ImU32 cellBorder = ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.85f, 0.6f));
			const ImU32 selectedBorder = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.95f));
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));

			const size_t slotCount = std::min(materials.size(), counts.size());
			for (size_t i = 0; i < slotCount; ++i)
			{
				ImVec2 cellMin = ImGui::GetCursorScreenPos();
				ImVec2 cellMax = ImVec2(cellMin.x + cellSize.x, cellMin.y + cellSize.y);

				drawList->AddRectFilled(cellMin, cellMax, cellFill, 4.0f);
				drawList->AddRect(cellMin, cellMax, (i == selectedIndex) ? selectedBorder : cellBorder, 4.0f, 0, (i == selectedIndex) ? 2.0f : 1.0f);

				unsigned int textureId = getInventoryTextureCache().getTextureId(materials[i]);
				if (textureId != 0)
				{
					ImVec2 imageMin(cellMin.x + imagePadding.x, cellMin.y + imagePadding.y);
					ImVec2 imageMax(cellMax.x - imagePadding.x, cellMax.y - imagePadding.y);
					ImTextureRef textureRef(static_cast<ImTextureID>(textureId));
					drawList->AddImage(textureRef, imageMin, imageMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				}

				char slotText[4];
				std::snprintf(slotText, sizeof(slotText), "%zu", i + 1);
				drawList->AddText(ImVec2(cellMin.x + 4.0f, cellMin.y + 2.0f), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.9f)), slotText);

				char countText[8];
				std::snprintf(countText, sizeof(countText), "%d", counts[i]);
				ImVec2 countSize = ImGui::CalcTextSize(countText);
				drawList->AddText(ImVec2(cellMax.x - countSize.x - 4.0f, cellMax.y - countSize.y - 2.0f), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.85f)), countText);

				ImGui::Dummy(cellSize);

				if (i + 1 < slotCount)
				{
					ImGui::SameLine();
				}
			}

			ImGui::PopStyleVar();
		}
		ImGui::End();
	}

	void showPoints(int points)
	{
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(12.0f, 68.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("PointsOverlay", nullptr, flags | ImGuiWindowFlags_NoBackground))
		{
			ImGui::Text("Points: %d", points);
		}
		ImGui::End();
	}

	void showTeamScoreBar(int playerPoints, int enemyPoints)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const float barHeight = 14.0f;
		const float windowWidth = 420.0f;
		const float windowHeight = barHeight + 4.0f;

		const float startX = viewport->WorkPos.x + (viewport->WorkSize.x - windowWidth) * 0.5f;

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(startX, viewport->WorkPos.y + margin), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("TeamScoreOverlay", nullptr, flags))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 barMin = ImGui::GetCursorScreenPos();
			ImVec2 barMax = ImVec2(barMin.x + windowWidth, barMin.y + barHeight);

			const int totalPoints = playerPoints + enemyPoints;
			const float playerFraction = (totalPoints > 0) ? (static_cast<float>(playerPoints) / static_cast<float>(totalPoints)) : 0.5f;
			const float splitX = barMin.x + windowWidth * playerFraction;

			const ImU32 barBackground = ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
			const ImU32 blueFill = ImGui::GetColorU32(ImVec4(0.2f, 0.45f, 0.9f, 0.85f));
			const ImU32 redFill = ImGui::GetColorU32(ImVec4(0.9f, 0.2f, 0.2f, 0.85f));
			const ImU32 border = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.35f));
			const ImU32 textColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.9f));

			drawList->AddRectFilled(barMin, barMax, barBackground, 4.0f);
			drawList->AddRectFilled(barMin, ImVec2(splitX, barMax.y), blueFill, 4.0f, ImDrawFlags_RoundCornersLeft);
			drawList->AddRectFilled(ImVec2(splitX, barMin.y), barMax, redFill, 4.0f, ImDrawFlags_RoundCornersRight);
			drawList->AddRect(barMin, barMax, border, 4.0f);

			char blueText[16];
			char redText[16];
			std::snprintf(blueText, sizeof(blueText), "%d", playerPoints);
			std::snprintf(redText, sizeof(redText), "%d", enemyPoints);

			ImVec2 blueSize = ImGui::CalcTextSize(blueText);
			ImVec2 redSize = ImGui::CalcTextSize(redText);

			const float blueCenterX = barMin.x + (splitX - barMin.x) * 0.5f;
			const float redCenterX = splitX + (barMax.x - splitX) * 0.5f;
			const float textY = barMin.y + (barHeight - blueSize.y) * 0.5f;

			drawList->AddText(ImVec2(blueCenterX - blueSize.x * 0.5f, textY), textColor, blueText);
			drawList->AddText(ImVec2(redCenterX - redSize.x * 0.5f, textY), textColor, redText);

			ImGui::Dummy(ImVec2(windowWidth, barHeight));
		}
		ImGui::End();
	}

	void showFpsOverlay(float fps, FpsCorner corner)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 10.0f;
		const bool  onRight  = corner == FpsCorner::TopRight  || corner == FpsCorner::BottomRight;
		const bool  onBottom = corner == FpsCorner::BottomLeft || corner == FpsCorner::BottomRight;
		const ImVec2 anchor(onRight  ? 1.0f : 0.0f, onBottom ? 1.0f : 0.0f);
		const ImVec2 pos(
			viewport->WorkPos.x + (onRight  ? viewport->WorkSize.x - margin : margin),
			viewport->WorkPos.y + (onBottom ? viewport->WorkSize.y - margin : margin));

		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, anchor);
		ImGui::SetNextWindowBgAlpha(0.6f);
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));
		if (ImGui::Begin("##FpsOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("FPS: %.0f", fps);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void showCrosshair(const GeneralSettings& gs)
	{
		if (!gs.showCrosshair) return;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImVec2 center(
			viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
			viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);

		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		const ImU32 col = ImGui::ColorConvertFloat4ToU32(
			ImVec4(gs.crosshairColor[0], gs.crosshairColor[1], gs.crosshairColor[2], gs.crosshairColor[3]));
		const float s   = gs.crosshairSize;
		const float t   = gs.crosshairThickness;

		// horizontal line
		drawList->AddLine(ImVec2(center.x - s, center.y), ImVec2(center.x + s, center.y), col, t);
		// vertucal line
		drawList->AddLine(ImVec2(center.x, center.y - s), ImVec2(center.x, center.y + s), col, t);
	}

	void showOverlayMessage(const char* message)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.7f);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("OverlayMessage", nullptr, flags))
		{
			ImGui::TextUnformatted(message);
		}
		ImGui::End();
	}

	bool showWaitingForPlayers(const std::function<void()>& onButtonClick)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(260.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.75f);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		bool cancelClicked = false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));
		if (ImGui::Begin("##WaitingOverlay", nullptr, flags))
		{
			ImGui::TextUnformatted("Waiting for players...");
			ImGui::Spacing();
			if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f)))
			{
				if (onButtonClick) onButtonClick();
				cancelClicked = true;
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
		return cancelClicked;
	}

	static bool s_menuOpen           = false;
	static bool s_changelogOpen      = false;
	static bool s_changelogNewContent = true;  // shown until user opens changelog once after game start
	static bool s_showAllChangelog   = false;

	static bool             s_settingsOpen        = false;
	static GeneralSettings  s_settingsSnapshot    = {};
	static ControlsSettings s_controlsSnapshot    = {};
	static float            s_sensitivitySnapshot = 1.0f;

	void resetMainMenuState()
	{
		s_menuOpen      = false;
		s_changelogOpen = false;
	}

	bool isMainMenuPanelOpen()
	{
		return s_menuOpen;
	}


	// changelog
	void showChangelogPanel(const ImGuiViewport* viewport, const std::function<void()>& onButtonClick)
	{
		if (!s_changelogOpen) { s_showAllChangelog = false; return; }

		// when full changelog is open, hide the small panel
		if (!s_showAllChangelog)
		{
			const float changelogWidth = 260.0f;
			const float margin = 12.0f;
			const ImVec2 pos(viewport->WorkPos.x + viewport->WorkSize.x - margin,
				viewport->WorkPos.y + viewport->WorkSize.y - margin - 32.0f);
			ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
			ImGui::SetNextWindowSize(ImVec2(changelogWidth, 0.0f), ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.30f);
			if (ImGui::Begin("Changelog", nullptr,
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings))
			{
				constexpr int previewCount = 3;
				const int totalEntries = static_cast<int>(std::size(changelog));
				const int displayCount = totalEntries < previewCount ? totalEntries : previewCount;
				for (int i = 0; i < displayCount; ++i)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.35f, 1.0f));
					ImGui::TextUnformatted(changelog[i].version);
					ImGui::PopStyleColor();
					ImGui::TextWrapped("%s", changelog[i].changes);
					ImGui::Spacing();
				}
				if (totalEntries > previewCount)
				{
					ImGui::Separator();
					if (ImGui::Button("Show all changes", ImVec2(-1.0f, 0.0f)))
					{
						if (onButtonClick) onButtonClick();
						s_showAllChangelog = true;
					}
				}
			}
			ImGui::End();
		}

		// full changelog view
		if (s_showAllChangelog)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				s_showAllChangelog = false;
				s_changelogOpen    = false;
			}

			const float fullW = 400.0f;
			const float fullH = 500.0f;

			ImVec2 fullPos;
			ImVec2 fullPivot;
			if (s_menuOpen)
			{
				fullPos   = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f + 270.0f,
					viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
				fullPivot = ImVec2(0.0f, 0.5f);
			}
			else
			{
				fullPos   = viewport->GetCenter();
				fullPivot = ImVec2(0.5f, 0.5f);
			}

			ImGui::SetNextWindowPos(fullPos, ImGuiCond_Always, fullPivot);
			ImGui::SetNextWindowSize(ImVec2(fullW, fullH), ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.92f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
			if (ImGui::Begin("Full Changelog", &s_showAllChangelog,
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::BeginChild("##FullChangelogScroll", ImVec2(0.0f, fullH - 80.0f), false);
				for (const auto& entry : changelog)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.35f, 1.0f));
					ImGui::TextUnformatted(entry.version);
					ImGui::PopStyleColor();
					ImGui::TextWrapped("%s", entry.changes);
					ImGui::Spacing();
				}
				ImGui::EndChild();

				ImGui::Separator();
				if (ImGui::Button("Close", ImVec2(fullW - 32.0f, 0.0f)))
				{
					if (onButtonClick) onButtonClick();
					s_showAllChangelog = false;
					s_changelogOpen    = false;
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
			if (!s_showAllChangelog)
				s_changelogOpen = false;
		}
	}

	void showGameVersion(const char* version, bool changelogAllowed, const std::function<void()>& onButtonClick)
	{
		if (!changelogAllowed)
			s_changelogOpen = false;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const ImVec2 bottomRight =
			ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - margin,
				viewport->WorkPos.y + viewport->WorkSize.y - margin);

		ImGui::SetNextWindowBgAlpha(0.65f);
		ImGui::SetNextWindowPos(bottomRight, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.12f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1,1,1,0.20f));
		if (ImGui::Begin("GameVersionOverlay", nullptr, flags | ImGuiWindowFlags_NoBackground))
		{
			if (s_changelogNewContent)
				{
					ImGui::SetWindowFontScale(1.6f);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.15f, 0.15f, 1.0f));
					ImGui::TextUnformatted("!");
					ImGui::PopStyleColor();
					ImGui::SetWindowFontScale(1.0f);
					ImGui::SameLine(0.0f, 4.0f);
				}
			if (ImGui::Button(version, ImVec2(0.0f, 0.0f)))
			{
				if (onButtonClick) onButtonClick();
				s_changelogOpen = !s_changelogOpen;
				if (s_changelogOpen) s_changelogNewContent = false;
			}
		}
		ImGui::End();
		ImGui::PopStyleColor(3);

		showChangelogPanel(viewport, onButtonClick);
	}

	bool showMainMenu(bool& flyMode, int& platformOffsetBlocks, int& platformSize, bool& multiplayerEnabled, int& networkRole, char* serverAddress, size_t serverAddressSize, int& serverPort, const std::vector<SavedServerView>& savedServers, ServerListResult& serverListResult, char* newServerName, size_t newServerNameSize, const char* errorMessage, PlayerSettings& playerSettings, WorldSettings& worldSettings, MatchSettings& matchSettings, const bool slotExists[3], SettingsSlotResult& slotResult, bool& outSettingsClicked, bool& outQuitClicked, const std::function<void()>& onButtonClick)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImVec2 center = viewport->GetCenter();

		outSettingsClicked = false;
		outQuitClicked     = false;

		if (!s_menuOpen)
		{
			const float marginLeft = 60.0f;
			const float marginTop  = 80.0f;

			// Title
			ImGui::SetNextWindowPos(
				ImVec2(viewport->WorkPos.x + marginLeft, viewport->WorkPos.y + marginTop),
				ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if (ImGui::Begin("##Title", nullptr,
				ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::SetWindowFontScale(2.0f);
				ImGui::TextUnformatted("Peaces Skyland");
				ImGui::SetWindowFontScale(1.0f);
				ImGui::PopStyleColor();
			}
			ImGui::End();
			ImGui::PopStyleVar();

			// Buttons
			const float btnSpacing = 18.0f;
			ImGui::SetNextWindowPos(
				ImVec2(viewport->WorkPos.x + marginLeft, viewport->WorkPos.y + marginTop + 90.0f),
				ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, btnSpacing));
			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.12f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1, 1, 1, 0.20f));
			ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			if (ImGui::Begin("##MainBtns", nullptr,
				ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::SetWindowFontScale(1.3f);
				if (ImGui::Button("Play"))
				{
					if (onButtonClick) onButtonClick();
					s_menuOpen = true;
				}
				if (ImGui::Button("Settings"))
				{
					if (onButtonClick) onButtonClick();
					outSettingsClicked = true;
				}
				if (ImGui::Button("Quit"))
				{
					if (onButtonClick) onButtonClick();
					outQuitClicked = true;
				}
				ImGui::SetWindowFontScale(1.0f);
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(4);
			return false;
		}

		// Full menu
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			s_menuOpen = false;

		static int  activeMenuTab = 0; // 0=Game, 1=World, 2=Multiplayer
		static bool devSettings   = false;

		const float menuW = 520.0f;
		const float menuH = 460.0f;
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(menuW, menuH), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.18f);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize   |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar;

		serverListResult = {};
		slotResult       = {};

		bool playClicked = false;
		if (ImGui::Begin("##NewGame", nullptr, flags))
		{
			// Tab bar
			const float tabW = (menuW - 24.0f) / 3.0f;
			const char* tabLabels[] = { "Game", "World", "Multiplayer" };
			for (int i = 0; i < 3; ++i)
			{
				if (i > 0) ImGui::SameLine(0.0f, 4.0f);
				const bool sel = (activeMenuTab == i);
				if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
				if (ImGui::Button(tabLabels[i], ImVec2(tabW, 30.0f)))
					{
						if (onButtonClick) onButtonClick();
						activeMenuTab = i;
					}
				if (sel) ImGui::PopStyleColor();
			}
			ImGui::Separator();

			ImGui::BeginChild("##MenuContent", ImVec2(0.0f, menuH - 100.0f), false);

			if (activeMenuTab == 0) // GAME
			{
				{
					char lbl[32];
					std::snprintf(lbl, sizeof(lbl), "Fly Mode: %s", flyMode ? "ON" : "OFF");
					if (ImGui::Button(lbl, ImVec2(-1.0f, 30.0f)))
					{
						if (onButtonClick) onButtonClick();
						flyMode = !flyMode;
					}
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::TextUnformatted("Match");
				ImGui::Spacing();

				const int maxDuration = devSettings ? 300 : 30;
				ImGui::SliderInt("Duration (min)", &matchSettings.matchDurationMinutes, 1, maxDuration);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Duration of the match in minutes\nDefault: 5");

				const int totalBlocks = platformSize * platformSize * worldSettings.platformBaseHeight;
				const int maxMinBlocks = devSettings
					? std::max(1, static_cast<int>(totalBlocks * 0.99f))
					: std::max(1, static_cast<int>(totalBlocks * 0.75f));
				matchSettings.minBlocksForLoss = std::clamp(matchSettings.minBlocksForLoss, 0, maxMinBlocks);
				ImGui::SliderInt("Min blocks for loss", &matchSettings.minBlocksForLoss, 0, maxMinBlocks);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Opponent loses when their block count drops below this\nDefault: 10");

				const int maxInitialBlocks = devSettings ? 1000 : 50;
				matchSettings.initialBlockCount = std::clamp(matchSettings.initialBlockCount, 0, maxInitialBlocks);
				ImGui::SliderInt("Initial blocks per type", &matchSettings.initialBlockCount, 0, maxInitialBlocks);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("How many blocks of each type each player starts with\nDefault: 10");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::TextUnformatted("Presets");
				ImGui::Spacing();
				for (int i = 0; i < 3; ++i)
				{
					ImGui::PushID(i);
					char lbl[16];
					std::snprintf(lbl, sizeof(lbl), "Slot %d", i + 1);
					ImGui::TextUnformatted(lbl);
					ImGui::SameLine();
					if (ImGui::SmallButton("Save")) { if (onButtonClick) onButtonClick(); slotResult.saveSlot = i; }
					ImGui::SameLine();
					if (!slotExists[i]) ImGui::BeginDisabled();
					if (ImGui::SmallButton("Load")) { if (onButtonClick) onButtonClick(); slotResult.loadSlot = i; }
					if (!slotExists[i]) ImGui::EndDisabled();
					ImGui::PopID();
				}
			}
			else if (activeMenuTab == 1) // WORLD
			{
				ImGui::Checkbox("Dev settings", &devSettings);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Unlocks extended slider ranges for all settings");
				ImGui::Spacing();

				const int maxDistance = devSettings ? 500 : 100;
				const int maxSize     = devSettings ? 100 : 20;
				if (!devSettings)
				{
					platformOffsetBlocks = std::clamp(platformOffsetBlocks, 10, maxDistance);
					platformSize         = std::clamp(platformSize, 2, maxSize);
				}
				ImGui::SliderInt("Distance between platforms", &platformOffsetBlocks, 10, maxDistance);
				ImGui::SliderInt("Platform size",              &platformSize,         2,  maxSize);

				const int maxBaseH = devSettings ? 100 : 10;
				ImGui::SliderInt("Platform base height", &worldSettings.platformBaseHeight, 1, maxBaseH);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Solid foundation layers per platform\nDefault: 5");

				const int maxBuildH = devSettings ? 500 : 20;
				const int minBuildH = devSettings ? 1   : 6;
				ImGui::SliderInt("Max build height", &worldSettings.maxBuildHeightBlocks, minBuildH, maxBuildH);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Total build-height grid\nDefault: 8");

				ImGui::Separator();
				ImGui::TextUnformatted("Player");
				ImGui::Spacing();

				const float maxMoveSpeed = devSettings ? 100.0f : 10.0f;
				const float minMoveSpeed = devSettings ? 0.01f  : 0.1f;
				ImGui::SliderFloat("Move speed", &playerSettings.moveSpeed, minMoveSpeed, maxMoveSpeed, "%.2f");
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Base movement speed\nDefault: 5.0");

				if (flyMode)
				{
					ImGui::SliderFloat("Fly speed (H)", &playerSettings.flySpeedHorizontal,
						devSettings ? 0.01f  : 0.1f,  devSettings ? 50.0f : 5.0f,  "%.2f");
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
						ImGui::SetTooltip("Horizontal speed multiplier in fly mode\nDefault: 1.0");

					ImGui::SliderFloat("Fly speed (V)", &playerSettings.flySpeedVertical,
						devSettings ? 0.001f : 0.025f, devSettings ? 5.0f  : 0.5f,  "%.4f");
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
						ImGui::SetTooltip("Vertical speed in fly mode\nDefault: 0.05");

					ImGui::SliderFloat("Jet max speed", &playerSettings.jetMaxSpeed,
						devSettings ? 0.001f : 0.05f,  devSettings ? 10.0f : 1.0f,  "%.2f");
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
						ImGui::SetTooltip("Max upward jet speed\nDefault: 0.3");
				}

				ImGui::SliderFloat("Ball speed", &playerSettings.ballSpeed,
					devSettings ? 0.1f : 0.5f, devSettings ? 200.0f : 20.0f, "%.1f");
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Projectile launch speed\nDefault: 3.0");
			}
			else //  MULTIPLAYER
			{
				ImGui::Checkbox("Enable Multiplayer", &multiplayerEnabled);
				if (multiplayerEnabled)
				{
					ImGui::Spacing();
					ImGui::RadioButton("Host (Server)", &networkRole, 0);
					ImGui::RadioButton("Client",        &networkRole, 1);
					ImGui::InputInt("Port", &serverPort);

					if (networkRole == 1)
					{
						ImGui::InputText("Server Address", serverAddress, serverAddressSize);

						ImGui::Separator();
						ImGui::TextUnformatted("Saved Servers");
						ImGui::Spacing();

						if (savedServers.empty())
						{
							ImGui::TextDisabled("No saved servers");
						}
						else if (ImGui::BeginTable("ServerList", 3))
						{
							ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthStretch);
							ImGui::TableSetupColumn("Sel",  ImGuiTableColumnFlags_WidthFixed);
							ImGui::TableSetupColumn("Del",  ImGuiTableColumnFlags_WidthFixed);
							for (size_t i = 0; i < savedServers.size(); ++i)
							{
								ImGui::PushID(static_cast<int>(i));
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("%s (%s:%d)", savedServers[i].name, savedServers[i].address, savedServers[i].port);
								ImGui::TableSetColumnIndex(1);
								if (ImGui::SmallButton("Select")) serverListResult.selectIndex = static_cast<int>(i);
								ImGui::TableSetColumnIndex(2);
								if (ImGui::SmallButton("X"))      serverListResult.removeIndex = static_cast<int>(i);
								ImGui::PopID();
							}
							ImGui::EndTable();
						}

						ImGui::Separator();
						ImGui::InputText("Name", newServerName, newServerNameSize);
						if (ImGui::Button("Save Server"))
						{
							if (onButtonClick) onButtonClick();
							serverListResult.saveRequested = true;
						}
					}
				}
			}

			if (errorMessage && errorMessage[0] != '\0')
			{
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				ImGui::TextWrapped("%s", errorMessage);
				ImGui::PopStyleColor();
			}

			ImGui::EndChild();

			ImGui::Separator();
			ImGui::Spacing();
			const float btnW = (menuW - 24.0f - 4.0f) * 0.5f;
			playClicked = ImGui::Button("Play", ImVec2(btnW, 32.0f));
			if (playClicked && onButtonClick) onButtonClick();
			ImGui::SameLine(0.0f, 4.0f);
			if (ImGui::Button("Cancel", ImVec2(btnW, 32.0f)))
			{
				if (onButtonClick) onButtonClick();
				s_menuOpen = false;
			}
		}
		ImGui::End();

		if (playClicked)
			s_menuOpen = false;
		return playClicked;
	}

	EscapeMenuResult showEscapeMenu(bool isInGame, float& mouseSensitivity, GeneralSettings& generalSettings, ControlsSettings& controlsSettings, const std::function<void()>& onButtonClick, bool allowSettings)
	{
		EscapeMenuResult result{};
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowBgAlpha(0.55f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
		ImGui::Begin("##EscMenuOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::End();
		ImGui::PopStyleColor();

		const bool settingsJustOpened = (generalSettings.settingsOpen && !s_settingsOpen);
		if (settingsJustOpened)
		{
			s_settingsSnapshot    = generalSettings;
			s_controlsSnapshot    = controlsSettings;
			s_sensitivitySnapshot = mouseSensitivity;
			s_settingsOpen        = true;
		}

		// revert settings if left without saving
		auto revertSettings = [&]()
		{
			if (generalSettings.vsync          != s_settingsSnapshot.vsync)          result.vsyncChanged    = true;
			if (generalSettings.masterVolume   != s_settingsSnapshot.masterVolume)   result.volumeChanged   = true;
			if (generalSettings.windowWidth    != s_settingsSnapshot.windowWidth  ||
				generalSettings.windowHeight   != s_settingsSnapshot.windowHeight ||
				generalSettings.isFullscreen   != s_settingsSnapshot.isFullscreen)   result.resizeRequested = true;

			generalSettings  = s_settingsSnapshot;
			controlsSettings = s_controlsSnapshot;
			mouseSensitivity = s_sensitivitySnapshot;
			s_settingsOpen   = false;
		};

		const float panelWidth = 200.0f;
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 60.0f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f),
			ImGuiCond_Always, ImVec2(0.0f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(panelWidth, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 18.0f));
		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.12f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1,1,1,0.20f));
		ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f,1.0f,1.0f,1.0f));

		if (ImGui::Begin("##EscapeMenu", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetWindowFontScale(1.3f);
			const ImVec2 buttonSize(0.0f, 0.0f);

			const bool buttonsDisabled = s_settingsOpen;

			const bool hideMainMenuButtons = !isInGame && s_settingsOpen;

			if (buttonsDisabled && !hideMainMenuButtons) ImGui::BeginDisabled();

			if (!hideMainMenuButtons)
			{
				if (ImGui::Button("Resume", buttonSize))
				{
					if (onButtonClick) onButtonClick();
					s_settingsOpen = false;
					generalSettings.settingsOpen = false;
					result.playContinueClicked = true;
				}
			}

			if (isInGame || allowSettings)
			{
				if (ImGui::Button("Settings", buttonSize))
				{
					if (onButtonClick) onButtonClick();
					s_settingsOpen = !s_settingsOpen;
					generalSettings.settingsOpen = s_settingsOpen;
				}
			}

			if (buttonsDisabled && !hideMainMenuButtons) ImGui::EndDisabled();

			if (s_settingsOpen && ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				revertSettings();
				generalSettings.settingsOpen = false;
			}

			// disconnect (only in game)
			if (isInGame)
			{
				if (buttonsDisabled) ImGui::BeginDisabled();
				if (ImGui::Button("Disconnect", buttonSize))
				{
					if (onButtonClick) onButtonClick();
					result.disconnectClicked = true;
				}
				if (buttonsDisabled) ImGui::EndDisabled();
			}

			// quit
			if (!hideMainMenuButtons)
			{
				if (buttonsDisabled) ImGui::BeginDisabled();
				if (ImGui::Button("Quit", buttonSize))
				{
					if (onButtonClick) onButtonClick();
					result.exitClicked = true;
				}
				if (buttonsDisabled) ImGui::EndDisabled();
			}
			ImGui::SetWindowFontScale(1.0f);
		}
		ImGui::End();
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(2);

		// settings
		if (s_settingsOpen)
		{
			const float settingsW = 480.0f;
			const float settingsH = 400.0f;
			ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
				ImGui::SetNextWindowSize(ImVec2(settingsW, settingsH), ImGuiCond_Always);
				ImGui::SetNextWindowBgAlpha(0.97f);
				ImGui::SetNextWindowFocus();

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

				static int activeTab = 0; // 0=Video, 1=Audio, 2=Controls

				if (ImGui::Begin("##SettingsPopup", nullptr,
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoMove))
			{
				// Title
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.85f, 0.7f, 1.0f));
				ImGui::SetWindowFontScale(1.1f);
				ImGui::TextUnformatted("SETTINGS");
				ImGui::SetWindowFontScale(1.0f);
				ImGui::PopStyleColor();
				ImGui::Separator();

				// Tab bar
				const char* tabs[] = { "VIDEO", "AUDIO", "CONTROLS" };
				for (int i = 0; i < 3; ++i)
				{
					if (i > 0) ImGui::SameLine();
					const bool selected = (activeTab == i);
					if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
					if (ImGui::Button(tabs[i], ImVec2(140.0f, 26.0f)))
					{
						if (onButtonClick) onButtonClick();
						activeTab = i;
					}
					if (selected) ImGui::PopStyleColor();
				}
				ImGui::Separator();

				// Tab content
				ImGui::BeginChild("##SettingsContent", ImVec2(0.0f, settingsH - 140.0f), false);

				if (activeTab == 0) // VIDEO
				{
					ImGui::TextUnformatted("Mouse");
					ImGui::SetNextItemWidth(settingsW - 48.0f);
					ImGui::SliderFloat("##MouseSens", &mouseSensitivity, 0.1f, 5.0f, "Sensitivity %.2f");

					ImGui::Separator();
					ImGui::TextUnformatted("Screen");

					struct Preset { const char* label; int w; int h; };
					static const Preset presets[] = {
						{ "1280x720",  1280,  720 },
						{ "1600x900",  1600,  900 },
						{ "1920x1080", 1920, 1080 },
						{ "2560x1440", 2560, 1440 },
					};

					GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
					const GLFWvidmode* vidMode = primaryMonitor ? glfwGetVideoMode(primaryMonitor) : nullptr;
					const int monitorW = vidMode ? vidMode->width  : 99999;
					const int monitorH = vidMode ? vidMode->height : 99999;

					const float halfBtn = (settingsW - 48.0f - 4.0f) * 0.5f;
					for (int i = 0; i < 4; ++i)
					{
						const bool exceedsMonitor = presets[i].w > monitorW || presets[i].h > monitorH;
						if (exceedsMonitor) ImGui::BeginDisabled();
						if (ImGui::Button(presets[i].label, ImVec2(halfBtn, 0.0f)))
						{
							if (onButtonClick) onButtonClick();
							if (generalSettings.isFullscreen)
							{
								result.renderResolutionChanged = true;
								result.renderWidth  = presets[i].w;
								result.renderHeight = presets[i].h;
							}
							else
							{
								generalSettings.windowWidth  = presets[i].w;
								generalSettings.windowHeight = presets[i].h;
								result.resizeRequested = true;
							}
						}
						if (exceedsMonitor) ImGui::EndDisabled();
						if (i % 2 == 0) ImGui::SameLine(0.0f, 4.0f);
					}

					if (ImGui::Checkbox("Fullscreen", &generalSettings.isFullscreen))
						result.resizeRequested = true;

					if (ImGui::Checkbox("VSync", &generalSettings.vsync))
						result.vsyncChanged = true;

					ImGui::Separator();
					ImGui::TextUnformatted("FPS Counter");
					ImGui::Checkbox("Show FPS", &generalSettings.showFps);
					if (generalSettings.showFps)
					{
						static const char* cornerNames[] = { "Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right" };
						int cornerIdx = static_cast<int>(generalSettings.fpsCorner);
						ImGui::SetNextItemWidth(settingsW - 48.0f);
						if (ImGui::Combo("Position##FpsCorner", &cornerIdx, cornerNames, 4))
							generalSettings.fpsCorner = static_cast<FpsCorner>(cornerIdx);
					}

					ImGui::Separator();
						ImGui::TextUnformatted("Crosshair");
						ImGui::Checkbox("##ShowCrosshair", &generalSettings.showCrosshair);
						ImGui::SameLine(); ImGui::TextUnformatted("Show");
						if (generalSettings.showCrosshair)
						{
							const float ctrlW = settingsW - 48.0f;
							const float halfW = (ctrlW - 4.0f) * 0.5f;
							ImGui::SetNextItemWidth(halfW);
							ImGui::SliderFloat("##CrosshairSize", &generalSettings.crosshairSize, 4.0f, 30.0f, "Size %.0f");
							ImGui::SameLine(0.0f, 4.0f);
							ImGui::SetNextItemWidth(halfW);
							ImGui::SliderFloat("##CrosshairThick", &generalSettings.crosshairThickness, 0.5f, 5.0f, "Width %.1f");
							ImGui::SetNextItemWidth(ctrlW);
									ImGui::ColorEdit4("##CrosshairColor", generalSettings.crosshairColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
										ImGui::SameLine(); ImGui::TextUnformatted("Color");
									}

								ImGui::Separator();
								ImGui::TextUnformatted("Clouds");
								ImGui::SetNextItemWidth(settingsW - 48.0f);
								ImGui::SliderFloat("##CloudDist", &generalSettings.cloudDrawDistance, 500.0f, 9500.0f, "Draw Distance %.0f");
							}
							else if (activeTab == 1) // AUDIO
				{
					ImGui::TextUnformatted("Master Volume");
					float volumePct = generalSettings.masterVolume * 100.0f;
					ImGui::SetNextItemWidth(settingsW - 48.0f);
					if (ImGui::SliderFloat("##MasterVolume", &volumePct, 0.0f, 100.0f, "Volume %.0f%%"))
					{
						generalSettings.masterVolume = volumePct / 100.0f;
						result.volumeChanged = true;
					}
				}
				else if (activeTab == 2) // CONTROLS
				{
					static int rebindingAction = -1;

					struct ActionDef { const char* label; int* key; };
					ActionDef actions[] = {
						{ "Move Forward",   &controlsSettings.keyForward   },
						{ "Move Back",      &controlsSettings.keyBack      },
						{ "Move Left",      &controlsSettings.keyLeft      },
						{ "Move Right",     &controlsSettings.keyRight     },
						{ "Jump / Jet",     &controlsSettings.keyJump      },
						{ "Fly Toggle",     &controlsSettings.keyFlyToggle },
						{ "Build Mode",     &controlsSettings.keyBuildMode },
						{ "Camera Follow",  &controlsSettings.keyCamFollow },
					};
					constexpr int actionCount = 8;

					// cancel rebind if Escape pressed
					if (rebindingAction >= 0 && ImGui::IsKeyPressed(ImGuiKey_Escape))
						rebindingAction = -1;

						if (rebindingAction >= 0)
						{
							for (int k = ImGuiKey_Space; k < ImGuiKey_COUNT; ++k)
							{
								if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k)))
								{
								*actions[rebindingAction].key = k;
								result.controlsChanged = true;
								rebindingAction = -1;
								break;
							}
						}
					}

					if (ImGui::BeginTable("##ControlsTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
					{
						ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 140.0f);
						ImGui::TableHeadersRow();

						for (int i = 0; i < actionCount; ++i)
						{
							ImGui::PushID(i);
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TextUnformatted(actions[i].label);
							ImGui::TableSetColumnIndex(1);

							char keyLabel[32];
							if (rebindingAction == i)
							{
								std::snprintf(keyLabel, sizeof(keyLabel), "[ Press a key... ]");
							}
							else
							{
								const char* name = ImGui::GetKeyName(static_cast<ImGuiKey>(*actions[i].key));
								std::snprintf(keyLabel, sizeof(keyLabel), "%s", (name && name[0]) ? name : "?");
							}

							if (ImGui::Button(keyLabel, ImVec2(-1.0f, 0.0f)))
							{
								if (onButtonClick) onButtonClick();
								rebindingAction = (rebindingAction == i) ? -1 : i;
							}
							ImGui::PopID();
						}
						ImGui::EndTable();
					}

					if (rebindingAction >= 0)
					{
						ImGui::TextDisabled("Press a key to rebind, Escape to cancel");
					}
				}

				ImGui::EndChild();

				ImGui::Separator();

				const float btnW = (settingsW - 48.0f - 8.0f) / 3.0f;

				// buttons for settings: Reset, Save, Close

				if (ImGui::Button("Reset", ImVec2(btnW, 0.0f)))
				{
					if (onButtonClick) onButtonClick();
					if (activeTab == 0)
					{
						generalSettings.windowWidth   = 1280;
							generalSettings.windowHeight  = 720;
							generalSettings.isFullscreen  = false;
							generalSettings.vsync         = true;
							generalSettings.showFps       = false;
							generalSettings.fpsCorner     = FpsCorner::TopLeft;
							mouseSensitivity              = 1.0f;
							generalSettings.showCrosshair    = true;
							generalSettings.crosshairSize    = 10.0f;
							generalSettings.crosshairThickness = 1.5f;
							generalSettings.crosshairColor[0] = 1.0f;
							generalSettings.crosshairColor[1] = 1.0f;
							generalSettings.crosshairColor[2] = 1.0f;
							generalSettings.crosshairColor[3] = 0.9f;
							result.resizeRequested        = true;
							result.vsyncChanged           = true;
					}
					else if (activeTab == 1)
					{
						generalSettings.masterVolume = 0.5f;
						result.volumeChanged         = true;
					}
					else if (activeTab == 2)
					{
						controlsSettings = ControlsSettings{};
						result.controlsChanged = true;
					}
				}

				ImGui::SameLine(0.0f, 4.0f);

				if (ImGui::Button("Save", ImVec2(btnW, 0.0f)))
				{
					if (onButtonClick) onButtonClick();
					result.saveRequested         = true;
					s_settingsOpen               = false;
					generalSettings.settingsOpen = false;
				}

				ImGui::SameLine(0.0f, 4.0f);

				if (ImGui::Button("Close", ImVec2(btnW, 0.0f)))
				{
					if (onButtonClick) onButtonClick();
					revertSettings();
					generalSettings.settingsOpen = false;
					result.playContinueClicked   = true;
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
		return result;
	}

	void revertEscapeMenuSettings(float& mouseSensitivity, GeneralSettings& generalSettings, ControlsSettings& controlsSettings)
	{
		if (!s_settingsOpen) return;
		generalSettings  = s_settingsSnapshot;
		controlsSettings = s_controlsSnapshot;
		mouseSensitivity = s_sensitivitySnapshot;
		s_settingsOpen   = false;
		generalSettings.settingsOpen = false;
	}

	void showMatchTimer(float remainingMinutes)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float margin = 12.0f;
		const float scoreBarOffset = 22.0f;
		const ImVec2 topCenter =
			ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
				viewport->WorkPos.y + margin + scoreBarOffset);

		const float clamped = remainingMinutes < 0.0f ? 0.0f : remainingMinutes;
		const int mins = static_cast<int>(clamped);
		const int secs = static_cast<int>((clamped - static_cast<float>(mins)) * 60.0f);
		char timerText[16];
		std::snprintf(timerText, sizeof(timerText), "%d:%02d", mins, secs);

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(topCenter, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		if (ImGui::Begin("MatchTimerOverlay", nullptr, flags | ImGuiWindowFlags_NoBackground))
		{
			ImGui::TextUnformatted(timerText);
		}
		ImGui::End();
	}

	bool showMatchResult(bool playerWon, const std::function<void()>& onButtonClick)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowBgAlpha(0.55f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Begin("##MatchResultOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoInputs);
		ImGui::End();
		ImGui::PopStyleColor();

		const float panelWidth = 260.0f;
		ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(panelWidth, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.92f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 14.0f));

		bool mainMenuClicked = false;
		if (ImGui::Begin("##MatchResult", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			const char* resultText = playerWon ? "You won!" : "You lost!";
			const ImVec4 color = playerWon
				? ImVec4(0.25f, 0.95f, 0.35f, 1.0f)
				: ImVec4(0.95f, 0.25f, 0.25f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::SetWindowFontScale(1.8f);
			const ImVec2 textSize = ImGui::CalcTextSize(resultText);
			ImGui::SetCursorPosX((panelWidth - textSize.x) * 0.5f);
			ImGui::TextUnformatted(resultText);
			ImGui::SetWindowFontScale(1.0f);
			ImGui::PopStyleColor();

			ImGui::Spacing();
			const ImVec2 buttonSize(panelWidth - 40.0f, 36.0f);
			if (ImGui::Button("Main Menu", buttonSize))
			{
				if (onButtonClick) onButtonClick();
				mainMenuClicked = true;
			}
		}
		ImGui::End();

		ImGui::PopStyleVar(2);

		return mainMenuClicked;
	}
}
