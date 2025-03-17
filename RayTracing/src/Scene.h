#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 0.0f };        // ��������ɫ    Kd
	glm::vec3 SpecularColor{ 0.0f }; // ���淴����ɫ  Ks
	float Shininess = 1.0f;          // �����        Ns
	glm::vec3 TransmissionColor{ 1.0f }; // ͸����ɫ    Tr
	float RefractionIndex = 1.0f;    // ������        Ni

	float EmissionPower = 0.0f;		    // �����
	glm::vec3 EmissionColor{ 0.0f };	// ������ɫ

	std::string DiffuseTexture;      // ����������·��

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

	// ��ѡ�ļ��ٽṹ
// std::shared_ptr<BVHNode> BVHRoot; 
};