#include "ReadFiles.h"
#include <sstream>
using namespace tinyxml2;


bool ReadFiles::LoadXml(const std::string& path)
{
    // ��XML�ļ�
	XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << path << std::endl;
        std::cerr << "Error: " << doc.ErrorStr() << std::endl;
        return false;
    }

    // �������������
    XMLElement* cameraElem = doc.FirstChildElement("camera");
    if (cameraElem) {
        // ��ȡ����
        camera.type = cameraElem->Attribute("type");

        cameraElem->QueryIntAttribute("width", &camera.width);
        cameraElem->QueryIntAttribute("height", &camera.height);
        cameraElem->QueryFloatAttribute("fovy", &camera.fovy);

        // ��ȡ��Ԫ��eye
        XMLElement* eyeElem = cameraElem->FirstChildElement("eye");
        if (eyeElem) {
            eyeElem->QueryFloatAttribute("x", &camera.eye.x);
            eyeElem->QueryFloatAttribute("y", &camera.eye.y);
            eyeElem->QueryFloatAttribute("z", &camera.eye.z);
        }

        // ��ȡ��Ԫ��lookat
        XMLElement* lookatElem = cameraElem->FirstChildElement("lookat");
        if (lookatElem) {
            lookatElem->QueryFloatAttribute("x", &camera.lookat.x);
            lookatElem->QueryFloatAttribute("y", &camera.lookat.y);
            lookatElem->QueryFloatAttribute("z", &camera.lookat.z);
        }

        // ��ȡ��Ԫ��up
        XMLElement* upElem = cameraElem->FirstChildElement("up");
        if (upElem) {
            upElem->QueryFloatAttribute("x", &camera.up.x);
            upElem->QueryFloatAttribute("y", &camera.up.y);
            upElem->QueryFloatAttribute("z", &camera.up.z);
        }
    }
    else {
        std::cerr << "No camera element found!" << std::endl;
        return false;
    }
    // �����ƹ�����
    XMLElement* lightElem = doc.FirstChildElement("light");
    while (lightElem) {
        Read::Light light;
        light.mtlname = lightElem->Attribute("mtlname");
        const char* radianceStr = lightElem->Attribute("radiance");
        if (radianceStr) {
            std::istringstream iss(radianceStr);
            char comma;
            iss >> light.radiance[0] >> comma
                >> light.radiance[1] >> comma
                >> light.radiance[2];
        }
        lights.push_back(light);
        lightElem = lightElem->NextSiblingElement("light");
    }
    return true;
}

Material ReadFiles::ConvertAiMaterial(aiMaterial* aiMat)
{
    Material mat;
    aiColor3D color;
    aiString matName;

    // ��ȡ��������
    if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
        std::cout << "\n[��������]" << matName.C_Str() << std::endl;
    }

    // ������ɫ (Kd)
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        mat.Albedo = glm::vec3(color.r, color.g, color.b);
        std::cout << "  - Albedo: (" << mat.Albedo.r << ", " << mat.Albedo.g << ", " << mat.Albedo.b << ")\n";
    }

    // ���淴����ɫ (Ks)
    if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        mat.SpecularColor = glm::vec3(color.r, color.g, color.b);
        std::cout << "  - SpecularColor: (" << mat.SpecularColor.r << ", " << mat.SpecularColor.g << ", " << mat.SpecularColor.b << ")\n";
    }

    // ����� (Ns)
    float shininess;
    if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        mat.Shininess = shininess;
        std::cout << "  - Roughness: " << mat.Shininess << "\n";
    }

    // ͸����ɫ (Tr)
    if (aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, color) == AI_SUCCESS) {
        mat.TransmissionColor = glm::vec3(color.r, color.g, color.b);
        std::cout << "  - TransmissionColor: (" << mat.TransmissionColor.r << ", " << mat.TransmissionColor.g << ", " << mat.TransmissionColor.b << ")\n";
    }

    // ������ (Ni)
    float refraction;
    if (aiMat->Get(AI_MATKEY_REFRACTI, refraction) == AI_SUCCESS) {
        mat.RefractionIndex = refraction;
        std::cout << "  - RefractionIndex: " << mat.RefractionIndex << "\n";
    }

    // ���������� (map_Kd)
    aiString texturePath;
    if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
        mat.DiffuseTexture = texturePath.C_Str();
        std::cout << "  - DiffuseTexture: " << mat.DiffuseTexture << "\n";
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
    // ���ȼ������в���
    for (unsigned i = 0; i < aiScene->mNumMaterials; i++) {
        aiMaterial* aiMat = aiScene->mMaterials[i];
        Material material = ConvertAiMaterial(aiMat);

        // ����Ƿ���XML�ж���ķ������
        aiString matName;
        if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
            for (const auto& light : lights) {
                if (light.mtlname == matName.C_Str()) {
                    // ���÷�������
                    float maxRadiance = std::max({ light.radiance[0], light.radiance[1], light.radiance[2] });
                    material.EmissionColor = glm::normalize(glm::vec3(
                        light.radiance[0],
                        light.radiance[1],
                        light.radiance[2]
                    ));
                    material.EmissionPower = maxRadiance;  // ���÷���ǿ��Ϊ���
                    break;
                }
            }
        }
        scene.Materials.push_back(material);
    }
    // ������������
    for (unsigned i = 0; i < aiScene->mNumMeshes; i++) {
        aiMesh* aiMesh = aiScene->mMeshes[i];
        Mesh mesh;
        mesh.MaterialIndex = aiMesh->mMaterialIndex;  // ���ֲ��������Ĺ���

        // ����������
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

            tri.MaterialIndex = mesh.MaterialIndex;  // ȷ��������֪�����Ĳ���
            mesh.Triangles.push_back(tri);
        }

        scene.Meshes.push_back(mesh);
    }
}
