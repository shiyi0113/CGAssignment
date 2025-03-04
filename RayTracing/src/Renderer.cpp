#include "Renderer.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);
		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
	static uint32_t PCG_Hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}
	static float RandomFloat(uint32_t& seed)
	{
		seed = PCG_Hash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();
	}
	static glm::vec3 InUintSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f)
		);
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage) {
		// 长宽一样时无需改变
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();
	for (uint32_t y = 0;y < m_FinalImage->GetHeight();y++) {
		for (uint32_t x = 0;x < m_FinalImage->GetWidth();x++) {
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(ray);  // 返回该像素的颜色
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));  //将颜色的值限制在[0,1]内
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	glm::vec3 rayOrigin = ray.Origin;                  
	glm::vec3 rayDirection = ray.Direction;             
	float radius = 0.5f;                                     // 球的半径
	//球的方程：x^2+y^2+z^2=radius   球心坐标(0,0)
	//float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
	//float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
	//float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y + rayOrigin.z * rayOrigin.z -  radius * radius;
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float delta = b * b - 4.0f * a * c;
	if (delta < 0.0f)
		return glm::vec4(0, 0, 0, 1);//不在球范围内，染黑色

	float outPointT = (-b + glm::sqrt(delta)) / (2.0f * a);     // 出点的距离t
	float inPointT = (-b - glm::sqrt(delta)) / (2.0f * a);      // 入点的距离t
	glm::vec3 outPoint = rayOrigin + outPointT * rayDirection;  // 出点坐标
	glm::vec3 inPoint = rayOrigin + inPointT * rayDirection;    // 入点坐标
	glm::vec3 normal = glm::normalize(inPoint);                  // 法线

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));  // 光线方向(-1, -1, -1); 类似光源在(1,1,1)处
	float diffuse = glm::max(glm::dot(normal, -lightDir), 0.0f);  // 漫反射系数
	glm::vec3 sphereColor(1, 0, 1);							     // 球的颜色
	sphereColor *= diffuse;                                      // 漫反射颜色

	return glm::vec4(sphereColor, 1);   //在球范围内，染粉色
}
