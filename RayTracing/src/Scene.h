#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 0.0f };        // 漫反射颜色    Kd
	glm::vec3 SpecularColor{ 0.0f }; // 镜面反射颜色  Ks
	float Shininess = 1.0f;          // 光泽度        Ns
	glm::vec3 TransmissionColor{ 1.0f }; // 透射颜色    Tr
	float RefractionIndex = 1.0f;    // 折射率        Ni

	float EmissionPower = 0.0f;		    // 发光度
	glm::vec3 EmissionColor{ 0.0f };	// 发光颜色

	std::string DiffuseTexture;      // 漫反射纹理路径

	glm::vec3 GetEmission() const
	{
		return EmissionColor * EmissionPower;
	}
};

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
};

struct Triangle {
	Vertex V0, V1, V2;
	int MaterialIndex;
};

struct Mesh {
	std::vector<Triangle> Triangles;
	int MaterialIndex;
};

struct Scene
{
	std::vector<Material> Materials;
	std::vector<Mesh> Meshes;

	// 可选的加速结构
// std::shared_ptr<BVHNode> BVHRoot; 
};