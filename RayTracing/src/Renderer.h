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
	float HitDistance;               // ȷ���ĸ��ཻ��������ģ��Ӷ�������Ⱦ�ĸ�����
	glm::vec3 WorldPosition;         // �����������ཻ����������λ��
	glm::vec3 WorldNormal;           // �ཻ�㴦�������ķ�������
	int ObjectIndex;            // �ཻ�������ڳ����е�����
};
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height); //�ı�ͼ���С�봰��һ��
	void Render(const Scene& scene,const Camera& camera);   

	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // ��ȡ����ͼ��
private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);     // ����ÿ���������ɹ���
	HitPayload TraceRay(const Ray& ray);            // ���ݹ��߻�����ɫ  
	HitPayload ClosestHit(const Ray& ray, float hitDistance,int objectIndex);  // ������߻��е����������
	HitPayload Miss(const Ray& ray);                // ����δ���κ������ཻ�Ĵ���

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
};