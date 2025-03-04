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
	void Render();   // 渲染所有像素
	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // 获取最终图像
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
};