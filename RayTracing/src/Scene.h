#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 0.0f };   // 物体的基本颜色，光线在物体表面反射时的颜色
	float Roughness = 1.0f;     // 粗糙度。[0,1] 0表示完全光滑，1表示完全粗糙
	float Metallic = 0;		    // 金属度。[0,1] 0表示完全非金属，1表示完全金属.金属度越高，表面反射光线的能力越强，且反射光线的颜色与 Albedo 相同。
																	      //金属度越低，表面反射光线的能力越弱，且反射光线的颜色与光源颜色相同。
	float EmmisionPower = 0.0f;		    // 发光度。[0,1] 0表示不发光，1表示完全发光
	glm::vec3 EmmisionColor{ 0.0f };	// 发光颜色
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

	// 可选的加速结构
// std::shared_ptr<BVHNode> BVHRoot; 
};