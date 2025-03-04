#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "Camera.h"

struct Ray {
	glm::vec3 Origin;
	glm::vec3 Direction;
};
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height); //�ı�ͼ���С�봰��һ��
	void Render(const Camera &camera);   

	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // ��ȡ����ͼ��
private:
	glm::vec4 TraceRay(const Ray& ray);   // ���ٹ���  

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};