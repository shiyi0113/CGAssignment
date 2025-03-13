#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 0.0f };   // ����Ļ�����ɫ��������������淴��ʱ����ɫ
	float Roughness = 1.0f;     // �ֲڶȡ�[0,1] 0��ʾ��ȫ�⻬��1��ʾ��ȫ�ֲ�
	float Metallic = 0;		    // �����ȡ�[0,1] 0��ʾ��ȫ�ǽ�����1��ʾ��ȫ����.������Խ�ߣ����淴����ߵ�����Խǿ���ҷ�����ߵ���ɫ�� Albedo ��ͬ��
																	      //������Խ�ͣ����淴����ߵ�����Խ�����ҷ�����ߵ���ɫ���Դ��ɫ��ͬ��
	float EmmisionPower = 0.0f;		    // ����ȡ�[0,1] 0��ʾ�����⣬1��ʾ��ȫ����
	glm::vec3 EmmisionColor{ 0.0f };	// ������ɫ
	glm::vec3 GetEmission() const
	{
		return EmmisionColor * EmmisionPower;
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