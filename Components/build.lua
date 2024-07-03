project "Components"
    kind "StaticLib"
    language "C++"

    location "%{wks.location}/Components"
    staticruntime(srunt)
    files {
        "%{prj.location}/**.cpp",
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/**.hpp",
    }

    removefiles { "%{prj.location}/implot_demos/**.*" }

    includedirs { "%{prj.location}/", 
                  "%{prj.location}/include", 
                  "%{prj.location}/src", 
                  "%{wks.location}/Vendor/imgui",
                  "%{wks.location}/Vendor/imgui/backends",
                  "%{wks.location}/Vendor/imgui/misc/cpp",
                  "%{wks.location}/Vendor/implot",
                  "%{wks.location}/Vendor/libs/glfw/include",
                --   "%{wks.location}/Vendor/yaml-cpp/include",

    }
    targetdir ( "%{wks.location}/lib/" )
    objdir ( "%{wks.location}/obj/%{cfg.buildcfg}" )

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
        links {  "legacy_stdio_definitions", "opengl32", "glfw3", "imgui", "implot",
        -- "yaml-cpp"
    }