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

	void OnResize(uint32_t width, uint32_t height); //�ı�ͼ���С�봰��һ��
	void Render();   
	// void ResetFrameIndex() { m_FrameIndex = 1; } // ������Ⱦ
	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // ��ȡ����ͼ��
private:
	glm::vec4 PerPixel(glm::vec2 coord);  // ÿ�����ص���ɫ
	
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	//uint32_t m_FrameIndex = 1;
};