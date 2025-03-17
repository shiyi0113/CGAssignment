#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <atomic>
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
	int ObjectIndex;                 // �ཻ�������ڳ����е�����
	int MaterialIndex;			   // �ཻ������Ĳ�������
};

class Renderer
{
	struct Setting {
		bool Accumulate = true;  // �Ƿ��ۻ���Ⱦ���
	};

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height); //�ı�ͼ���С�봰��һ��
	void Render(const Scene& scene,const Camera& camera);   
	std::shared_ptr<Walnut::Image> GetFinalImage()const { return m_FinalImage; } // ��ȡ����ͼ��
	void ResetFrameIndex() { m_FrameIndex = 1; } // ����֡����
	Setting& GetSetting() { return m_Setting; }  // ��ȡSetting

private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);     // ����ÿ���������ɹ���
	HitPayload TraceRay(const Ray& ray);            // ���ݹ��߻�����ɫ  
	HitPayload ClosestTriangleHit(const Ray& ray, float hitDistance, int meshIndex);  // ������߻��е������������
	HitPayload Miss(const Ray& ray);                // ����δ���κ������ཻ�Ĵ���

	bool RayTriangleIntersect(const Ray& ray, const Triangle& triangle, float& t);
	glm::vec3 CalculateBarycentricCoord(const glm::vec3& point, const Triangle& triangle);
	glm::vec3  ApplyGammaCorrection(const glm::vec3& color, float gamma);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	glm::vec4* m_AccumulationData = nullptr;   // �Ѽ�������Ⱦ���
	int m_FrameIndex = 1;					   // ֡����

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
	Setting m_Setting;
	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVAerticalIter;
	std::atomic<uint32_t> m_PixelsCompleted{ 0 };
};