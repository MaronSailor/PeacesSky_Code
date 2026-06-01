#pragma once

#include "ui/engineGui.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

class MainMenu
{
public:
	MainMenu()
	{
		loadSavedServers();
		loadSettings();
	}

	bool render(bool& outSettingsClicked, bool& outQuitClicked, const char* errorMessage = nullptr, const std::function<void()>& onButtonClick = nullptr)
	{
		outSettingsClicked = false;
		outQuitClicked     = false;

		buildServerViews();

		bool slotExists[3] = {
			hasSettingsSlot(0),
			hasSettingsSlot(1),
			hasSettingsSlot(2)
		};
		CustomEngine::EngineGui::SettingsSlotResult slotResult{};
		CustomEngine::EngineGui::ServerListResult result{};
		bool playClicked = CustomEngine::EngineGui::showMainMenu(
			menuFlyMode, menuPlatformOffsetBlocks, menuPlatformSize,
			menuMultiplayerEnabled, menuNetworkRole,
			menuServerAddress.data(), menuServerAddress.size(),
			menuServerPort, serverViews, result,
			newServerName.data(), newServerName.size(),
			errorMessage,
			menuPlayerSettings, menuWorldSettings, menuMatchSettings,
			slotExists, slotResult, outSettingsClicked, outQuitClicked, onButtonClick);

		handleServerListResult(result);
		if (slotResult.saveSlot >= 0) saveSettingsSlot(slotResult.saveSlot);
		if (slotResult.loadSlot >= 0) loadSettingsSlot(slotResult.loadSlot);
		if (playClicked)
			saveGeneralSettings();
		return playClicked;
	}

	bool getFlyMode() const { return menuFlyMode; }
	int getPlatformOffsetBlocks() const { return menuPlatformOffsetBlocks; }
	int getPlatformSize() const { return menuPlatformSize; }
	bool isMultiplayerEnabled() const { return menuMultiplayerEnabled; }
	bool isHost() const { return menuNetworkRole == 0; }
	const char* getServerAddress() const { return menuServerAddress.data(); }
	int getServerPort() const { return menuServerPort; }

	const CustomEngine::EngineGui::PlayerSettings& getPlayerSettings() const { return menuPlayerSettings; }
	const CustomEngine::EngineGui::WorldSettings& getWorldSettings() const { return menuWorldSettings; }
	const CustomEngine::EngineGui::MatchSettings& getMatchSettings() const { return menuMatchSettings; }
	CustomEngine::EngineGui::GeneralSettings& getGeneralSettings() { return menuGeneralSettings; }
	CustomEngine::EngineGui::ControlsSettings& getControlsSettings() { return menuControlsSettings; }

	void saveSettings()
	{
		saveGeneralSettings();
	}

	void saveGeneralSettings()
	{
		std::ofstream file(settingsFilePath, std::ios::trunc);
		if (!file.is_open()) return;
		file << "windowWidth="    << menuGeneralSettings.windowWidth             << '\n';
		file << "windowHeight="   << menuGeneralSettings.windowHeight            << '\n';
		file << "fullscreen="     << (menuGeneralSettings.isFullscreen ? 1 : 0)  << '\n';
		file << "vsync="          << (menuGeneralSettings.vsync ? 1 : 0)         << '\n';
		file << "showFps="        << (menuGeneralSettings.showFps ? 1 : 0)       << '\n';
		file << "fpsCorner="      << static_cast<int>(menuGeneralSettings.fpsCorner) << '\n';
		file << "masterVolume="   << menuGeneralSettings.masterVolume            << '\n';
		file << "cloudDrawDist=" << menuGeneralSettings.cloudDrawDistance        << '\n';
		file << "ctrlForward="    << menuControlsSettings.keyForward             << '\n';
		file << "ctrlBack="       << menuControlsSettings.keyBack                << '\n';
		file << "ctrlLeft="       << menuControlsSettings.keyLeft                << '\n';
		file << "ctrlRight="      << menuControlsSettings.keyRight               << '\n';
		file << "ctrlJump="       << menuControlsSettings.keyJump                << '\n';
		file << "ctrlFly="        << menuControlsSettings.keyFlyToggle           << '\n';
		file << "ctrlBuild="      << menuControlsSettings.keyBuildMode           << '\n';
		file << "ctrlCam="        << menuControlsSettings.keyCamFollow           << '\n';
	}

	void resetGameSettings()
	{
		menuFlyMode              = false;
		menuPlatformOffsetBlocks = 50;
		menuPlatformSize         = 6;
		menuPlayerSettings       = CustomEngine::EngineGui::PlayerSettings{};
		menuWorldSettings        = CustomEngine::EngineGui::WorldSettings{};
		menuMatchSettings        = CustomEngine::EngineGui::MatchSettings{};
	}

	bool hasSettingsSlot(int slot) const
	{
		std::ifstream f(slotFilePath(slot));
		return f.is_open();
	}

	void saveSettingsSlot(int slot)
	{
		std::ofstream file(slotFilePath(slot), std::ios::trunc);
		if (!file.is_open()) return;
		file << "flyMode="        << (menuFlyMode ? 1 : 0)                       << '\n';
		file << "platformSize="   << menuPlatformSize                            << '\n';
		file << "platformOffset=" << menuPlatformOffsetBlocks                    << '\n';
		file << "moveSpeed="      << menuPlayerSettings.moveSpeed                << '\n';
		file << "flySpeedH="      << menuPlayerSettings.flySpeedHorizontal       << '\n';
		file << "flySpeedV="      << menuPlayerSettings.flySpeedVertical         << '\n';
		file << "jetMaxSpeed="    << menuPlayerSettings.jetMaxSpeed              << '\n';
		file << "ballSpeed="      << menuPlayerSettings.ballSpeed                << '\n';
		file << "maxBuildHeight=" << menuWorldSettings.maxBuildHeightBlocks      << '\n';
		file << "platformBase="   << menuWorldSettings.platformBaseHeight        << '\n';
		file << "matchDuration="  << menuMatchSettings.matchDurationMinutes      << '\n';
		file << "minBlocks="      << menuMatchSettings.minBlocksForLoss          << '\n';
		file << "initialBlocks="  << menuMatchSettings.initialBlockCount         << '\n';
	}

	void loadSettingsSlot(int slot)
	{
		std::ifstream file(slotFilePath(slot));
		if (!file.is_open()) return;
		std::string line;
		while (std::getline(file, line))
		{
			const auto sep = line.find('=');
			if (sep == std::string::npos) continue;
			const std::string key = line.substr(0, sep);
			const std::string val = line.substr(sep + 1);
			try
			{
				if      (key == "flyMode")        menuFlyMode                            = std::stoi(val) != 0;
				else if (key == "platformSize")   menuPlatformSize                       = std::stoi(val);
				else if (key == "platformOffset") menuPlatformOffsetBlocks               = std::stoi(val);
				else if (key == "moveSpeed")      menuPlayerSettings.moveSpeed           = std::stof(val);
				else if (key == "flySpeedH")      menuPlayerSettings.flySpeedHorizontal  = std::stof(val);
				else if (key == "flySpeedV")      menuPlayerSettings.flySpeedVertical    = std::stof(val);
				else if (key == "jetMaxSpeed")    menuPlayerSettings.jetMaxSpeed         = std::stof(val);
				else if (key == "ballSpeed")      menuPlayerSettings.ballSpeed           = std::stof(val);
				else if (key == "maxBuildHeight") menuWorldSettings.maxBuildHeightBlocks = std::stoi(val);
				else if (key == "platformBase")   menuWorldSettings.platformBaseHeight   = std::stoi(val);
				else if (key == "matchDuration")  menuMatchSettings.matchDurationMinutes = std::stoi(val);
					else if (key == "minBlocks")      menuMatchSettings.minBlocksForLoss     = std::stoi(val);
					else if (key == "initialBlocks")  menuMatchSettings.initialBlockCount    = std::stoi(val);
			}
			catch (...) {}
		}
	}

private:
	struct SavedServerData
	{
		std::string name;
		std::string address;
		int port = 28000;
	};

	void buildServerViews()
	{
		serverViews.clear();
		serverViews.reserve(savedServers.size());
		for (const auto& s : savedServers)
		{
			serverViews.push_back({ s.name.c_str(), s.address.c_str(), s.port });
		}
	}

	void handleServerListResult(const CustomEngine::EngineGui::ServerListResult& result)
	{
		if (result.selectIndex >= 0 && result.selectIndex < static_cast<int>(savedServers.size()))
		{
			const auto& entry = savedServers[result.selectIndex];
			menuServerAddress.fill('\0');
			std::snprintf(menuServerAddress.data(), menuServerAddress.size(), "%s", entry.address.c_str());
			menuServerPort = entry.port;
			newServerName.fill('\0');
			std::snprintf(newServerName.data(), newServerName.size(), "%s", entry.name.c_str());
		}

		if (result.removeIndex >= 0 && result.removeIndex < static_cast<int>(savedServers.size()))
		{
			savedServers.erase(savedServers.begin() + result.removeIndex);
			saveSavedServers();
		}

		if (result.saveRequested)
		{
			std::string name = newServerName.data();
			std::string address = menuServerAddress.data();
			if (!name.empty() && !address.empty())
			{
				bool found = false;
				for (auto& s : savedServers)
				{
					if (s.name == name)
					{
						s.address = address;
						s.port = menuServerPort;
						found = true;
						break;
					}
				}
				if (!found)
				{
					savedServers.push_back({ name, address, menuServerPort });
				}
				saveSavedServers();
			}
		}
	}

	void loadSavedServers()
	{
		savedServers.clear();
		std::ifstream file(serversFilePath);
		if (!file.is_open())
		{
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream stream(line);
			std::string name;
			std::string address;
			std::string portStr;
			if (std::getline(stream, name, '|') &&
				std::getline(stream, address, '|') &&
				std::getline(stream, portStr))
			{
				SavedServerData entry;
				entry.name = name;
				entry.address = address;
				try { entry.port = std::stoi(portStr); }
				catch (...) { entry.port = 28000; }
				savedServers.push_back(entry);
			}
		}
	}

	void saveSavedServers()
	{
		std::ofstream file(serversFilePath, std::ios::trunc);
		if (!file.is_open())
		{
			return;
		}

		for (const auto& s : savedServers)
		{
			file << s.name << '|' << s.address << '|' << s.port << '\n';
		}
	}

	static constexpr const char* serversFilePath  = "servers.cfg";
	static constexpr const char* settingsFilePath = "settings.cfg";

	static std::string slotFilePath(int slot)
	{
		return std::string("preset") + std::to_string(slot + 1) + ".cfg";
	}

	void loadSettings()
	{
		std::ifstream file(settingsFilePath);
		if (!file.is_open()) return;
		std::string line;
		while (std::getline(file, line))
		{
			const auto sep = line.find('=');
			if (sep == std::string::npos) continue;
			const std::string key = line.substr(0, sep);
			const std::string val = line.substr(sep + 1);
			try
			{
				if      (key == "windowWidth")    menuGeneralSettings.windowWidth        = std::stoi(val);
				else if (key == "windowHeight")   menuGeneralSettings.windowHeight       = std::stoi(val);
				else if (key == "fullscreen")     menuGeneralSettings.isFullscreen       = std::stoi(val) != 0;
				else if (key == "vsync")          menuGeneralSettings.vsync              = std::stoi(val) != 0;
				else if (key == "showFps")        menuGeneralSettings.showFps            = std::stoi(val) != 0;
				else if (key == "fpsCorner")      menuGeneralSettings.fpsCorner          = static_cast<CustomEngine::EngineGui::FpsCorner>(std::stoi(val));
				else if (key == "masterVolume")   menuGeneralSettings.masterVolume       = std::stof(val);
				else if (key == "cloudDrawDist")  menuGeneralSettings.cloudDrawDistance  = std::stof(val);
					else if (key == "ctrlForward")    menuControlsSettings.keyForward        = std::stoi(val);
					else if (key == "ctrlBack")       menuControlsSettings.keyBack           = std::stoi(val);
					else if (key == "ctrlLeft")       menuControlsSettings.keyLeft           = std::stoi(val);
				else if (key == "ctrlRight")      menuControlsSettings.keyRight          = std::stoi(val);
				else if (key == "ctrlJump")       menuControlsSettings.keyJump           = std::stoi(val);
				else if (key == "ctrlFly")        menuControlsSettings.keyFlyToggle      = std::stoi(val);
				else if (key == "ctrlBuild")      menuControlsSettings.keyBuildMode      = std::stoi(val);
				else if (key == "ctrlCam")        menuControlsSettings.keyCamFollow      = std::stoi(val);
			}
			catch (...) {}
		}
	}

	bool menuFlyMode = false;
	int menuPlatformOffsetBlocks = 50;
	int menuPlatformSize = 6;
	bool menuMultiplayerEnabled = false;
	int menuNetworkRole = 0;
	std::array<char, 64> menuServerAddress = { "127.0.0.1" };
	int menuServerPort = 28000;
	std::array<char, 32> newServerName = {};
	std::vector<SavedServerData> savedServers;
	std::vector<CustomEngine::EngineGui::SavedServerView> serverViews;

	CustomEngine::EngineGui::PlayerSettings menuPlayerSettings{};
	CustomEngine::EngineGui::WorldSettings  menuWorldSettings{};
	CustomEngine::EngineGui::MatchSettings  menuMatchSettings{};
	CustomEngine::EngineGui::GeneralSettings menuGeneralSettings{};
	CustomEngine::EngineGui::ControlsSettings menuControlsSettings{};
};