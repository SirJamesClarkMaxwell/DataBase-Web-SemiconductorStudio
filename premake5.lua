-- include "utils.lua"



workspace "Web-DataBase-SemiconductorStudio"
	configurations { "Debug", "Release" }
	platforms { "x86", "x86_64" }
	language "C++"
	cppdialect "C++latest"

	location "./"

	srunt = "off"


	include "./Components/build.lua"
	include "./Vendor/build.lua"
	include "./Vendor/yaml-cpp"
	-- include "./Vendor/curl/premake5.lua"
	include "./Vendor/cpr/premake5.lua"
	include "./Web-DataBase-SemiconductorStudio/build.lua"


