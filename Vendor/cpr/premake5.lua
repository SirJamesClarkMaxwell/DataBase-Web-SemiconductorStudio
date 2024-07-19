project "cpr"
	kind "StaticLib"
	language "C++"
	location "%{wks.location}/Vendor/cpr"

	include "%{prj.location}/../Vendor/curl/premake5.lua"
	files
	{
		-- "../%{prj.location}/curl/**.h",
		-- "../%{prj.location}/curl/curl**.h",
		
		"%{prj.location}/cpr/**.h",
		"%{prj.location}/cpr/**.cpp",
		"%{prj.location}/include/cpr/**.h"
	}
	
	includedirs
	{
		-- "../%{prj.location}/curl/**.h",
		-- "../%{prj.location}/curl/curl**.h",
		
		"%{prj.location}/include",
		"%{prj.location}/curl",
		"%{prj.location}/curl/curl"
	}
	
	targetdir ("%{wks.location}/lib/%{cfg.buildcfg}")
	objdir ("%{wks.location}/obj/%{cfg.buildcfg}")
	
	libdirs ( "%{wks.location}/lib" )

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"
		links{"curl"}
		
		filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"
		links{"curl"}
		
		filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		links{"curl"}
		
		filter "configurations:Release"
		runtime "Release"
		optimize "on"
		links{"curl"}
