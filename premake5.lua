-- include "utils.lua"



workspace "Web-DataBase-SemiconductorStudio"
	configurations { "Debug", "Release" }
	platforms { "x86", "x86_64" }
	language "C++"
	cppdialect "C++20"

	location "./"

	srunt = "off"


	include "./Components/build.lua"
	include "./Vendor/build.lua"
	include "./Web-DataBase-SemiconductorStudio/build.lua"
	include "./Vendor/yaml-cpp"


