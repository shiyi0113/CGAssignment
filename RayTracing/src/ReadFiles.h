#pragma once
#include <string>
#include <iostream>
#include "tinyxml2.h"
#include "Scene.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace Read {
    struct Camera {
        std::string type;
        int width, height;
        float fovy;
        struct {
            float x, y, z;
        } eye, lookat, up;
    };

    struct Light {
        std::string mtlname;
        float radiance[3];  // RGBπ‚«ø
    };
}

class ReadFiles {
public:
    bool LoadXml(const std::string& path);
    void LoadModel(Scene& scene, const std::string& path);
	Read::Camera getCamera() { return camera; }
	Read::Light getLight() { return light; }
private:
    Material ConvertAiMaterial(aiMaterial* aiMat);
private:
    Read::Camera camera;
    Read::Light light;
};

