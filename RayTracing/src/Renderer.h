#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Scene.h"


struct Ray {
	glm::vec3 Origin;
	glm::vec3 Direction;
};

struct HitPayload
{
	float HitDistance;               // 确定哪个相交点是最近的，从而决定渲染哪个物体
	glm::vec3 WorldPosition;         // 光线与物体相交的世界坐标位置
	glm::vec3 WorldNormal;           // 相交点处物体表面的法线向量
	int ObjectIndex;            // 相交的物体在场景中的索引
};
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height); //改变图像大小与窗口一致
	void Render(const Scene& scene,const Camera& camera);   

	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // 获取最终图像
private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);     // 根据每个像素生成光线
	HitPayload TraceRay(const Ray& ray);            // 根据光线绘制颜色  
	HitPayload ClosestHit(const Ray& ray, float hitDistance,int objectIndex);  // 处理光线击中的最近的物体
	HitPayload Miss(const Ray& ray);                // 光线未与任何物体相交的处理

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
};