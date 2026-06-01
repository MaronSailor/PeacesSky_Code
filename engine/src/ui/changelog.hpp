#pragma once

namespace CustomEngine::EngineGui
{
	struct ChangelogEntry
	{
		const char* version;
		const char* changes;
	};

	inline const ChangelogEntry changelog[] = {
		{
			"v0.2.6",
			"- Clouds"
		},
		{
			"v0.2.5",
			"- Complete menu redesign"
		},
		{
			"v0.2.4",
			"- Remade changelog panel\n"
			"- Changed spawn priority on blocks"
		},
		{
			"v0.2.3",
			"- Minor bug fixes"
		},
		{
			"v0.2.2",
			"- Changed settings' panel\n"
			"- Added control settings\n"
			"- Various optimisations and bug fixes"
		},
		{
			"v0.2.1",
			"- Added sounds to the game and sound settings"
		},
		{
			"v0.2.0",
			"- More fixes to optimisation\n"
		},
		{
			"v0.1.7",
			"- Added FPS counter\n"
			"- FlyMode changes"
		},
		{
			"v0.1.6",
			"- Client can rejoin after leaving the game\n"
			"- Animation for host waiting\n"
			"- Client get kicked when host leaves\n"
			"- Host can wait for new client after a disconnect\n"
			"- Minor bug fixes"
		},
		{
			"v0.1.5",
			"- Added presets to world creation\n"
			"- Fixed different character settings\n"
			"- Fixed the fullscreen mode"
		},
		{
			"v0.1.4",
			"- Fixed the fullscreen mode"
		},
		{
			"v0.1.3",
			"- Added changelog panel to the main menu\n"
			"- Match timer & win/loss system\n"
			"- Match duration settings\n"
			"- Settings persistence"
		},
	};
}
