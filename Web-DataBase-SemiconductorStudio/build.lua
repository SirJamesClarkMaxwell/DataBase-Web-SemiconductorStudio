project "Web-DataBase-SemiconductorStudio"
	kind "ConsoleApp"
    language "C++"

    pchheader "pch.hpp"
    pchsource "%{prj.location}/src/pch.cpp"

    location "%{wks.location}/Web-DataBase-SemiconductorStudio"
    staticruntime(srunt)
    files {
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/Vendor/nlohmann/**.hpp"

    }

    removefiles { "%{prj.location}/implot_demos/**.*" }

    includedirs { "%{prj.location}/include", 
                  "%{wks.location}/Vendor/imgui",
                  "%{wks.location}/Vendor/imgui/backends",
                  "%{wks.location}/Vendor/imgui/misc/cpp",
                  "%{wks.location}/Vendor/implot",
                  "%{wks.location}/Vendor/libs/glfw/include",
                  "%{wks.location}/Vendor/nlohmann",
                  "%{wks.location}/Components/include",
                  "%{wks.location}/Vendor/yaml-cpp/include",
                  "%{wks.location}/Vendor/yaml-cpp/include/yaml-cpp",

                  "%{wks.location}/Vendor/curl",
                  "%{wks.location}/Vendor/curl/curl",
                  "%{wks.location}/Vendor/cpr",
                  "%{wks.location}/Vendor/cpr/cpr",
                  "%{wks.location}/Vendor/cpr/includes/cpr"

    }
    targetdir ( "%{wks.location}/bin/%{prj.name}-%{cfg.buildcfg}" )
    objdir ( "%{wks.location}/obj/%{cfg.buildcfg}" )


    defines {
		"YAML_CPP_STATIC_DEFINE"
	}
    
    libdirs ( "%{wks.location}/lib" )


    filter { "system:windows", "platforms:x86" }
		libdirs { "%{wks.location}/Vendor/libs/glfw/lib-vc2010-32" }

	filter { "system:windows", "platforms:x86_64" }
        libdirs { "%{wks.location}/Vendor/libs/glfw/lib-vc2010-64" }

    filter { "configurations:Debug" }
        runtime "Debug"
    filter { "configurations:Release" }
        runtime "Release"

    filter { "system:windows" }
		ignoredefaultlibraries { "msvcrt" }
        links {  "legacy_stdio_definitions", "opengl32", "glfw3", "imgui", "implot", "Components", "yaml-cpp","cpr" }


