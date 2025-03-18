#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <cfloat>
#include <string>

struct Ray;
// AABB�ṹ
struct AABB {
	glm::vec3 min{ FLT_MAX };
	glm::vec3 max{ -FLT_MAX };

	void Expand(const glm::vec3& p) {
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	void Expand(const AABB& other) {
		min = glm::min(min, other.min);
		max = glm::max(max, other.max);
	}
    // �����ཻ����
    bool IntersectRay(const Ray& ray, float& tMin, float& tMax) const;
};

// BVH�ڵ�
struct BVHNode {
	AABB bounds;
	std::unique_ptr<BVHNode> left;
	std::unique_ptr<BVHNode> right;
	std::vector<size_t> triangleIndices;  // �洢����������
	bool isLeaf{ false };
};

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
	// ��ȡ��Χ��
	AABB GetBounds() const {
		AABB bounds;
		bounds.Expand(V0.Position);
		bounds.Expand(V1.Position);
		bounds.Expand(V2.Position);
		return bounds;
	}

	// ��ȡ����
	glm::vec3 GetCentroid() const {
		return (V0.Position + V1.Position + V2.Position) / 3.0f;
	}
};

struct Mesh {
	std::vector<Triangle> Triangles;
	int MaterialIndex;
	AABB bounds;  // mesh�İ�Χ��

	// ���°�Χ��
    void UpdateBounds();
};

struct Scene
{
	std::vector<Material> Materials;
	std::vector<Mesh> Meshes;

    std::unique_ptr<BVHNode> BVHRoot;  // BVH���ڵ�

    // ����BVH�ķ���
	void BuildBVH();

private:
    // �ݹ鹹��BVH�ڵ�
	std::unique_ptr<BVHNode> BuildBVHNode(std::vector<std::pair<size_t, size_t>>& triangles,size_t start, size_t end);
};