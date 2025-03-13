#include "Renderer.h"
#include <execution>
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
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVAerticalIter.resize(height);
	for (uint32_t i = 0;i < width;i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0;i < height;i++)
		m_ImageVAerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene,const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;
	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
#define Multithreading 1
#if Multithreading
	std::for_each(std::execution::par, m_ImageVAerticalIter.begin(), m_ImageVAerticalIter.end(),
		[&](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);                             // 返回该像素的颜色
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color; // 累加颜色
					glm::vec4 accumlatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumlatedColor /= (float)m_FrameIndex;  // 平均颜色,累加几次除以几，防止过亮
					accumlatedColor = glm::clamp(accumlatedColor, glm::vec4(0.0f), glm::vec4(1.0f));  // 将颜色的值限制在[0,1]内
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumlatedColor);
				});
		});
#else
	for (uint32_t y = 0;y < m_FinalImage->GetHeight();y++) {
		for (uint32_t x = 0;x < m_FinalImage->GetWidth();x++) {
			glm::vec4 color = PerPixel(x, y);                             // 返回该像素的颜色
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color; // 累加颜色
			glm::vec4 accumlatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumlatedColor /= (float)m_FrameIndex;  // 平均颜色,累加几次除以几，防止过亮
			accumlatedColor = glm::clamp(accumlatedColor, glm::vec4(0.0f), glm::vec4(1.0f));  // 将颜色的值限制在[0,1]内
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumlatedColor);
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);
	if (m_Setting.Accumulate)
		m_FrameIndex++;
	else 
		m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x,uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 light{ 0.0f };
	glm::vec3 multiplier{ 1.0f };
	int bounces = 5;
	// 第一次是源光线，后面是反射的光线
	for (int i = 0;i < bounces;i++) {
		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			// light += skyColor * multiplier;
			break;
		}
		Sphere sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		Material material = m_ActiveScene->Materials[sphere.MaterialIndex];

		multiplier *= material.Albedo;
		light += material.GetEmission() * multiplier;
		// 改变光线
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f; //在击中点上加上一个微小的偏移
		//ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));   // 反射光线+粗糙度
		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
	}
	return glm::vec4(light, 1.0f);
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closeSphereIndex = -1;                           
	float closestT = std::numeric_limits<float>::max();  // 最近的距离

	for (size_t i = 0;i < m_ActiveScene->Spheres.size();i++) {
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 Origin = ray.Origin - sphere.Position;      // 根据球心坐标反向移动光线的起点            
		float radius = sphere.Radius;                                     // 球的半径
		//球的方程：x^2+y^2+z^2=radius   球心坐标(0,0)
		//float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
		//float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
		//float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y + rayOrigin.z * rayOrigin.z -  radius * radius;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(Origin, ray.Direction);
		float c = glm::dot(Origin, Origin) - radius * radius;
		float delta = b * b - 4.0f * a * c;

		if (delta < 0.0f)
			continue;  // 未击中这个球，继续下一个球
		float inPointT = (-b - glm::sqrt(delta)) / (2.0f * a);      // 入点的距离t
		if (inPointT > 0.0f && inPointT < closestT) {
			closestT = inPointT;
			closeSphereIndex = (int)i;
		}
	}

	if(closeSphereIndex == -1)
		return Miss(ray);       
	return ClosestHit(ray, closestT, closeSphereIndex);  // 击中了球 
}

HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;
	const Sphere& closeSphere = m_ActiveScene->Spheres[objectIndex];
	glm::vec3 Origin = ray.Origin - closeSphere.Position;
	payload.WorldPosition = Origin + hitDistance * ray.Direction;    // 入点坐标
	payload.WorldNormal = glm::normalize(payload.WorldPosition);     // 法线
	payload.WorldPosition += closeSphere.Position;                   // 世界坐标

	return payload;
}

HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
