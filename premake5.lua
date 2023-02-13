-- premake5.lua
workspace "binimg"
   configurations { "Debug", "Release" }

project "binimg"
    kind "ConsoleApp"
    language "C++"

	targetdir "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"

    files { "src/**.h", "src/**.cpp" }

    includedirs "vendor"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"