-- premake5.lua
workspace "RayTracing"
   architecture "x64"
   configurations { "Debug", "Release"}
   startproject "RayTracing"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "RayTracing"