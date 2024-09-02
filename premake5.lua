-- include "utils.lua"



workspace "Web-DataBase-SemiconductorStudio"
	configurations { "Debug", "Release" }
	platforms { "x86", "x86_64" }
	language "C++"
	cppdialect "C++latest"
	staticruntime "off"
	runtime "Debug"
	location "./"

	srunt = "off"


	include "./Components/build.lua"
	include "./Vendor/build.lua"
	include "./Vendor/yaml-cpp"
	-- include "./Vendor/LambertW/premake5.lua"
	-- include "./Vendor/curl/premake5.lua"
	-- include "./Vendor/cpr"
	include "./Web-DataBase-SemiconductorStudio/build.lua"
	include "./Vendor/NumericStorm/NumericStorm"


