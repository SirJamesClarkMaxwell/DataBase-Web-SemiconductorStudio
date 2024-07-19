project "curl"
    kind "StaticLib"
    language "C++"

    location "%{wks.location}/Vendor/curl"
    staticruntime(srunt)
    files {
        "%{prj.location}/**.h",
        "%{prj.location}/curl/**.h"
    }




    includedirs { 
		"%{prj.location}/", 
		"%{prj.location}/curl/", 

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
