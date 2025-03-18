#pragma once
#include <string>
#include <iostream>
#include <vector>
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
    std::vector<Read::Light> getLight() { return lights; }
    void SetSceneFolderPath(const std::string& path) { m_SceneFolderPath = path; }
private:
    Material ConvertAiMaterial(aiMaterial* aiMat);
private:
    Read::Camera camera;
    std::vector<Read::Light> lights;
    std::string m_SceneFolderPath;
};

