workspace "WEEK3TEAM7"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    platforms { "x86", "x64" }
    startproject "WEEK3TEAM7"

project "WEEK3TEAM7"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    characterset "Unicode"

    targetdir ("bin/%{cfg.platform}/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.platform}/%{cfg.buildcfg}")
    debugdir "%{wks.location}"

    files {
        "**.cpp",
        "**.h",
    }

    includedirs {
        ".",
        "ImGui",
    }

    links {
        "d3d11",
        "d3dcompiler",
        "dwrite",
        "d2d1",
        "dxgi",
        "dwmapi",
        "gdi32",
        "imm32",
        "user32"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Speed"
        symbols "On"

    filter "system:windows"
        defines {
            "UNICODE",
            "_UNICODE",
            "WIN32_LEAN_AND_MEAN",
        }

    filter "toolset:msc*"
    buildoptions { "/utf-8" }

    filter {}