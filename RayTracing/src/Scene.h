#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <cfloat>
#include <string>

struct Ray;
// AABB结构
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
    // 光线相交测试
    bool IntersectRay(const Ray& ray, float& tMin, float& tMax) const;
};

// BVH节点
struct BVHNode {
	AABB bounds;
	std::unique_ptr<BVHNode> left;
	std::unique_ptr<BVHNode> right;
	std::vector<size_t> triangleIndices;  // 存储三角形索引
	bool isLeaf{ false };
};

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
	// 获取包围盒
	AABB GetBounds() const {
		AABB bounds;
		bounds.Expand(V0.Position);
		bounds.Expand(V1.Position);
		bounds.Expand(V2.Position);
		return bounds;
	}

	// 获取质心
	glm::vec3 GetCentroid() const {
		return (V0.Position + V1.Position + V2.Position) / 3.0f;
	}
};

struct Mesh {
	std::vector<Triangle> Triangles;
	int MaterialIndex;
	AABB bounds;  // mesh的包围盒

	// 更新包围盒
    void UpdateBounds();
};

struct Scene
{
	std::vector<Material> Materials;
	std::vector<Mesh> Meshes;

    std::unique_ptr<BVHNode> BVHRoot;  // BVH根节点

    // 构建BVH的方法
	void BuildBVH();

private:
    // 递归构建BVH节点
	std::unique_ptr<BVHNode> BuildBVHNode(std::vector<std::pair<size_t, size_t>>& triangles,size_t start, size_t end);
};