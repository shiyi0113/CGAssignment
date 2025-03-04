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

void Renderer::Render()
{
	for (uint32_t y = 0;y < m_FinalImage->GetHeight();y++) {
		for (uint32_t x = 0;x < m_FinalImage->GetWidth();x++) {
			glm::vec2 coord = { (float)x / m_FinalImage->GetWidth() ,(float)y / m_FinalImage->GetHeight() }; // 归一化坐标 [0,1]
			coord = coord * 2.0f - 1.0f;    // 坐标映射到[-1,1]
			glm::vec4 color = PerPixel(coord); //返回该像素的颜色
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));  //将颜色的值限制在[0,1]内
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

// 传入像素的标准化坐标
glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);                   // 光线位置在屏幕外，z轴正方向上
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);         // 光线方向,从屏幕外射向屏幕上的该像素点
	float radius = 0.5f;                                     // 球的半径
	//球的方程：x^2+y^2+z^2=radius   球心坐标(0,0)
	//float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
	//float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
	//float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y + rayOrigin.z * rayOrigin.z -  radius * radius;
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float delta = b * b - 4.0f * a * c;
	if (delta >= 0.0f)
		return glm::vec4(1, 0, 1, 1);   //在球范围内，染粉色
	return glm::vec4(0, 0, 0, 1);     //不在球范围内，染黑色
}