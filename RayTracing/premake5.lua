project "RayTracing"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp", "src/**.c" }

   includedirs
   {
      "../Walnut/vendor/imgui",
      "../Walnut/vendor/glfw/include",
      "../Walnut/vendor/glm",
      "../Walnut/vendor/assimp/include",
      "../Walnut/Walnut/src",

      "%{IncludeDir.VulkanSDK}",
   }
   libdirs
   {
      "../Walnut/vendor/assimp/lib"
   }
   links
   {
       "Walnut",
       "assimp-vc143-mt",
   }
   postbuildcommands
   {
      '{COPY} "../Walnut/vendor/assimp/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
   }
   
   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"
