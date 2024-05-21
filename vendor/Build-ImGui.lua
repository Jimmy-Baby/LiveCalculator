project "imgui"
	filename "imgui/imgui"
	kind "StaticLib"
	language "C++"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files -- Files paths are relative to the PROJECT files path. The same goes for include dirs, and the output directories above.
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"backends/imgui_impl_win32.cpp",
		"backends/imgui_impl_win32.h",
		"backends/imgui_impl_dx12.cpp",
		"backends/imgui_impl_dx12.h",
		"misc/cpp/imgui_stdlib.cpp",
		"misc/cpp/imgui_stdlib.h"
	}
	
	includedirs
	{
		"./",
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"