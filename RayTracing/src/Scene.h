#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <cfloat>
#include <string>
#include "stb_image.h"
#include <iostream>
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

	std::string DiffuseTexturePath;      // 漫反射纹理路径
	std::vector<uint8_t> DiffuseTextureData; // 漫反射纹理数据
	int TextureWidth, TextureHeight, TextureChannels;

	glm::vec3 GetEmission() const
	{
		return EmissionColor * EmissionPower;
	}
	bool Material::LoadTexture(const std::string& sceneFolderPath)
	{
		std::string fullPath = sceneFolderPath + "/" + DiffuseTexturePath;
		std::cout << "Loading texture: " << fullPath << std::endl;
		if (DiffuseTexturePath.empty())
			return false;

		uint8_t* data = stbi_load(fullPath.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, 3);
		if (data)
		{
			DiffuseTextureData.assign(data, data + TextureWidth * TextureHeight * 3);
			stbi_image_free(data);
			std::cout << "Texture loaded successfully: " << fullPath << std::endl;
			std::cout << "Texture size: " << TextureWidth << "x" << TextureHeight << " Channels: " << TextureChannels << std::endl;
			std::cout << "First pixel data: " << (int)DiffuseTextureData[0] << ", " << (int)DiffuseTextureData[1] << ", " << (int)DiffuseTextureData[2] << std::endl;
			return true;
		}
		std::cerr << "Failed to load texture: " << fullPath << std::endl;
		return false;
	}

	glm::vec3 Material::SampleTexture(const glm::vec2& uv) const
	{
		if (DiffuseTextureData.empty())
		{
			std::cerr << "DiffuseTextureData is empty, returning Albedo" << std::endl;
			return Albedo;
		}

		int x = static_cast<int>(uv.x * TextureWidth) % TextureWidth;
		int y = static_cast<int>(uv.y * TextureHeight) % TextureHeight;

		// 确保 x 和 y 是非负数
		if (x < 0) x += TextureWidth;
		if (y < 0) y += TextureHeight;

		int index = (x + y * TextureWidth) * 3;

		if (index < 0 || index >= DiffuseTextureData.size())
		{
			std::cerr << "Index out of range: " << index << std::endl;
			std::cerr << "uv: (" << uv.x << ", " << uv.y << "), x: " << x << ", y: " << y << std::endl;
			std::cerr << "TextureWidth: " << TextureWidth << ", TextureHeight: " << TextureHeight << std::endl;
			return Albedo;
		}

		return glm::vec3(
			DiffuseTextureData[index] / 255.0f,
			DiffuseTextureData[index + 1] / 255.0f,
			DiffuseTextureData[index + 2] / 255.0f
		);
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