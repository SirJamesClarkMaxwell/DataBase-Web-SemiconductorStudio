project "cpr"
	kind "StaticLib"
	language "C++"
	location "%{wks.location}/Vendor/cpr"

	targetdir ("%{wks.location}/lib/%{cfg.buildcfg}")
	objdir ("%{wks.location}/obj/%{cfg.buildcfg}")

	files
	{
		"%{prj.location}/curl/**.h",
		"%{prj.location}/curl/curl**.h",

		"%{prj.location}/cpr/**.h",
		"%{prj.location}/cpr/**.cpp",
		"%{prj.location}/include/cpr/**.h"
	}

	includedirs
	{
		"%{prj.location}/include",
		"%{prj.location}/curl",
		"%{prj.location}/curl/curl"
	}

	-- defines
	-- {
	-- 	"YAML_CPP_STATIC_DEFINE"
	-- }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"