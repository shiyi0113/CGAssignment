#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 0.0f };   // ����Ļ�����ɫ��������������淴��ʱ����ɫ
	float Roughness = 1.0f;    // �ֲڶȡ�[0,1] 0��ʾ��ȫ�⻬��1��ʾ��ȫ�ֲ�
	float Metallic = 0;		// �����ȡ�[0,1] 0��ʾ��ȫ�ǽ�����1��ʾ��ȫ����.������Խ�ߣ����淴����ߵ�����Խǿ���ҷ�����ߵ���ɫ�� Albedo ��ͬ��
																	      //������Խ�ͣ����淴����ߵ�����Խ�����ҷ�����ߵ���ɫ���Դ��ɫ��ͬ��
};

struct Sphere
{
	glm::vec3 Position{ 0.0f };
	float Radius = 1;
	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};