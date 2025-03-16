#include "ReadFiles.h"
#include <sstream>
using namespace tinyxml2;


bool ReadFiles::LoadXml(const std::string& path)
{
    // 读XML文件
	XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << path << std::endl;
        std::cerr << "Error: " << doc.ErrorStr() << std::endl;
        return false;
    }
    // 获取根元素
    XMLElement* root = doc.RootElement();
    if (!root) {
        std::cerr << "No root element!" << std::endl;
        return false;
    }
    // 验证根元素名称
    if (strcmp(root->Name(), "scene") != 0) {
        std::cerr << "Root element must be 'scene', found: " << root->Name() << std::endl;
        return false;
    }
    // 解析摄像机配置
    XMLElement* cameraElem = root->FirstChildElement("camera");
    if (cameraElem) {
        // 读取属性
        camera.type = cameraElem->Attribute("type");

        cameraElem->QueryIntAttribute("width", &camera.width);
        cameraElem->QueryIntAttribute("height", &camera.height);
        cameraElem->QueryFloatAttribute("fovy", &camera.fovy);

        // 读取子元素eye
        XMLElement* eyeElem = cameraElem->FirstChildElement("eye");
        if (eyeElem) {
            eyeElem->QueryFloatAttribute("x", &camera.eye.x);
            eyeElem->QueryFloatAttribute("y", &camera.eye.y);
            eyeElem->QueryFloatAttribute("z", &camera.eye.z);
        }

        // 读取子元素lookat
        XMLElement* lookatElem = cameraElem->FirstChildElement("lookat");
        if (lookatElem) {
            lookatElem->QueryFloatAttribute("x", &camera.lookat.x);
            lookatElem->QueryFloatAttribute("y", &camera.lookat.y);
            lookatElem->QueryFloatAttribute("z", &camera.lookat.z);
        }

        // 读取子元素up
        XMLElement* upElem = cameraElem->FirstChildElement("up");
        if (upElem) {
            upElem->QueryFloatAttribute("x", &camera.up.x);
            upElem->QueryFloatAttribute("y", &camera.up.y);
            upElem->QueryFloatAttribute("z", &camera.up.z);
        }
    }
    // 解析灯光配置
    XMLElement* lightElem = root->FirstChildElement("light");
    if (lightElem) {
        light.mtlname = lightElem->Attribute("mtlname");
        const char* radianceStr = lightElem->Attribute("radiance");
        if (radianceStr) {
            std::istringstream iss(radianceStr);
            char comma;
            iss >> light.radiance[0] >> comma
                >> light.radiance[1] >> comma
                >> light.radiance[2];
        }
    }
    return true;
}

Material ReadFiles::ConvertAiMaterial(aiMaterial* aiMat)
{
    Material mat;
    aiColor3D color;
    aiString matName;

    // 获取材质名称
    if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
        std::cout << "\n[材质名称]" << matName.C_Str() << std::endl;
        // 检查是否是XML中定义的发光材质
        if (!light.mtlname.empty() && light.mtlname == matName.C_Str()) {
            float maxRadiance = std::max({ light.radiance[0], light.radiance[1], light.radiance[2] });
            mat.EmissionColor = glm::normalize(glm::vec3(light.radiance[0], light.radiance[1], light.radiance[2]));
            mat.EmissionPower = maxRadiance;
            std::cout << "  - 发光材质: 强度=" << mat.EmissionPower
                << ", 颜色=(" << mat.EmissionColor.r << ", "
                << mat.EmissionColor.g << ", " << mat.EmissionColor.b << ")\n";
        }
    }

    // 基础颜色
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        mat.Albedo = glm::vec3(color.r, color.g, color.b);
        std::cout << "  - Albedo: (" << mat.Albedo.r << ", "
            << mat.Albedo.g << ", " << mat.Albedo.b << ")\n";
    }
    else {
        std::cout << "  - Albedo: 未找到，使用默认值\n";
    }

    // 粗糙度
    float shininess;
    if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        mat.Roughness = 1.0f - glm::clamp(shininess / 256.0f, 0.0f, 1.0f);
    }

    // 金属度
    float metallic;
    if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        mat.Metallic = metallic;
    }

    return mat;
}

void ReadFiles::LoadModel(Scene& scene, const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs);
    if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return;
    }
    // 首先加载所有材质
    for (unsigned i = 0; i < aiScene->mNumMaterials; i++) {
        aiMaterial* aiMat = aiScene->mMaterials[i];
        Material material = ConvertAiMaterial(aiMat);

        // 检查是否是XML中定义的发光材质
        aiString matName;
        if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
            if (light.mtlname == matName.C_Str()) {
                // 设置发光属性
                float maxRadiance = std::max({ light.radiance[0], light.radiance[1], light.radiance[2] });
                material.EmissionColor = glm::normalize(glm::vec3(
                    light.radiance[0],
                    light.radiance[1],
                    light.radiance[2]
                ));
                material.EmissionPower = maxRadiance;  // 设置发光强度为最大
            }
        }

        scene.Materials.push_back(material);
    }
    // 加载所有网格
    for (unsigned i = 0; i < aiScene->mNumMeshes; i++) {
        aiMesh* aiMesh = aiScene->mMeshes[i];
        Mesh mesh;
        mesh.MaterialIndex = aiMesh->mMaterialIndex;  // 保持材质索引的关联

        // 处理顶点数据...
        for (unsigned j = 0; j < aiMesh->mNumFaces; j++) {
            aiFace& face = aiMesh->mFaces[j];
            Triangle tri;

            for (int k = 0; k < 3; k++) {
                Vertex& v = (&tri.V0)[k];
                const aiVector3D& pos = aiMesh->mVertices[face.mIndices[k]];
                v.Position = glm::vec3(pos.x, pos.y, pos.z);

                if (aiMesh->HasNormals()) {
                    const aiVector3D& norm = aiMesh->mNormals[face.mIndices[k]];
                    v.Normal = glm::vec3(norm.x, norm.y, norm.z);
                }

                if (aiMesh->HasTextureCoords(0)) {
                    const aiVector3D& uv = aiMesh->mTextureCoords[0][face.mIndices[k]];
                    v.TexCoord = glm::vec2(uv.x, uv.y);
                }
            }

            tri.MaterialIndex = mesh.MaterialIndex;  // 确保三角形知道它的材质
            mesh.Triangles.push_back(tri);
        }

        scene.Meshes.push_back(mesh);
    }
}
