#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height); //改变图像大小与窗口一致
	void Render();   
	// void ResetFrameIndex() { m_FrameIndex = 1; } // 重新渲染
	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // 获取最终图像
private:
	glm::vec4 PerPixel(glm::vec2 coord);  // 每个像素的颜色
	
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	//uint32_t m_FrameIndex = 1;
};