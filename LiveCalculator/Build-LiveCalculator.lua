project "LiveCalculator"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	targetdir "bin/%{cfg.buildcfg}"
	staticruntime "off"

	files { "src/**.h", "src/**.cpp" }

	includedirs
	{
		"./src",
		"../vendor/imgui",
    }

    links { "imgui", "d3d12", "dxgi", "dxguid", "d3dcompiler" }
	
	pchheader "Pch.h"
	pchsource "src/Pch.cpp"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	filter "system:windows"
		systemversion "latest"
		defines { "WL_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "WL_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "WL_RELEASE" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Dist"
		kind "WindowedApp"
		defines { "WL_DIST" }
		runtime "Release"
		optimize "On"
		symbols "Off"