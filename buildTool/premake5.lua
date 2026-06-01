workspace "CustomGameEngine" -- project name
	location ".."
	platforms { "x64", "x86", "linux-x64", "linux-x86" }

	configurations
	{
		"Debug",
		"Release",
		"Dist" -- not necessary, but nice to have
	}

	filter { "platforms:x86" }    architecture "x86"
	filter { "platforms:x64" }    architecture "x64"
	filter { "platforms:linux-x86" } architecture "x86"
	filter { "platforms:linux-x64" } architecture "x64"

	filter "system:windows"
		startproject "Sandbox"

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" -- declared as a variable

project "Engine" -- project name
	location "../engine" -- path to directory
	kind "SharedLib" -- to .dll
	language "C++"

	targetdir ("%{prj.location}/../bin/" .. outputDir .. "/%{prj.name}") -- compile to
	objdir ("%{prj.location}/../bin-int/" .. outputDir .. "/%{prj.name}") -- .obj file to


	files
	{
		"%{prj.location}/src/**.hpp",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/src/**.h"
	}


	includedirs
	{
		"%{prj.location}/dependencies/glew/include",
		"%{prj.location}/dependencies/glfw/include",
		"%{prj.location}/dependencies/glm/glm",
		"%{prj.location}/dependencies/stb",
		"%{prj.location}/dependencies/tinyobjloader"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"CE_PLATFORM_WINDOWS",
			"CE_BUILD_DLL",
			"GLEW_STATIC"
		}

		libdirs
		{
			"%{prj.location}/dependencies/glew/lib/Release/x64",
			"%{prj.location}/dependencies/glfw/lib-vc2022"
		}

		links
		{
			"opengl32.lib",
			"glew32s.lib",
			"glfw3_mt.lib"
		}

		postbuildcommands
		{
			("{MKDIR} ../bin/" .. outputDir .. "/Sandbox"),
			("{MKDIR} ../bin/" .. outputDir .. "/Game"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Sandbox"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Game")
		} 
		
	filter "system:linux"
		toolset "gcc"
		cppdialect "C++17"
		staticruntime "Off"
		pic "On"

		defines 
		{
			"PLATFORM_LINUX",
			"CE_PLATFORM_LINUX"
		}
		--add threading when needed

		postbuildcommands
		{
			("{MKDIR} ../bin/" .. outputDir .. "/Sandbox"),
			("{MKDIR} ../bin/" .. outputDir .. "/Game"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Sandbox"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Game")
		}

	filter "configurations:Debug"
		defines "ENGINE_DEBUG"
		symbols "On"
		buildoptions "/MDd" --needs the linux multithread flag!!!

	filter "configurations:Release"
		defines "ENGINE_RELEASE"
		optimize "On"
		buildoptions "/MD"

	filter "configurations:Dist"
		defines "ENGINE_DIST"
		optimize "On"
		buildoptions "/MD"


project "Sandbox"
	location "../sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("%{prj.location}/../bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{prj.location}/../bin-int/" .. outputDir .. "/%{prj.name}")

	files
	{
		"%{prj.location}/src/**.hpp",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/src/**.h"
	}

	includedirs
	{
		"../engine/src"
	}

	dependson
	{
		"Engine"
	}

	links
	{
		"Engine"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"CE_PLATFORM_WINDOWS"
		} 

	filter "system:linux"
		toolset "gcc"
		cppdialect "C++17"
		staticruntime "off"
		pic "On"
		
		defines
		{
			"PLATFORM_LINUX",
			"CE_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "ENGINE_DEBUG"
		symbols "On"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines "ENGINE_RELEASE"
		optimize "On"
		buildoptions "/MD"

	filter "configurations:Dist"
		defines "ENGINE_DIST"
		optimize "On"
		buildoptions "/MD"


project "Game"
	location "../game"
	kind "WindowedApp" -- "ConsoleApp" -- also possible
	language "C++"

	targetdir ("%{prj.location}/../bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{prj.location}/../bin-int/" .. outputDir .. "/%{prj.name}")

	files
	{
		"%{prj.location}/src/**.hpp",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/src/**.h"
	}

	includedirs
	{
		"../engine/src"
	}

	dependson
	{
		"Engine"
	}

	links
	{
		"Engine"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"CE_PLATFORM_WINDOWS",
			"CE_WINDOWS_NO_CONSOLE"
		} 

	filter "system:linux"
		toolset "gcc"
		cppdialect "C++17"
		staticruntime "off"
		pic "On"
		
		defines
		{
			"PLATFORM_LINUX",
			"CE_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "ENGINE_DEBUG"
		symbols "On"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines "ENGINE_RELEASE"
		optimize "On"
		buildoptions "/MD"

	filter "configurations:Dist"
		defines "ENGINE_DIST"
		optimize "On"
		buildoptions "/MD"


-- for now only win/linux support mac comming in a upcomming version
--also if needed, change toolset gcc to any other compiler