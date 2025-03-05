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
		// ����һ��ʱ����ı�
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

void Renderer::Render(const Scene& scene,const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	for (uint32_t y = 0;y < m_FinalImage->GetHeight();y++) {
		for (uint32_t x = 0;x < m_FinalImage->GetWidth();x++) {
			glm::vec4 color = PerPixel(x, y);                             // ���ظ����ص���ɫ
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));  // ����ɫ��ֵ������[0,1]��
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(uint32_t x,uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];
	glm::vec3 color{ 0.0f,0.0f,0.0f };
	float multiplier = 1.0f;
	int bounces = 2;
	// ��һ����Դ���ߣ��ڶ����Ƿ���Ĺ���
	for (int i = 0;i < bounces;i++) {
		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.0f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));  // ���߷���(-1, -1, -1); ���ƹ�Դ��(1,1,1)��
		float diffuse = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // ������ϵ��
		glm::vec3 sphereColor = m_ActiveScene->Spheres[payload.ObjectIndex].Color;					 // �����ɫ
		sphereColor *= diffuse;                                      // ��������ɫ
		color += sphereColor* multiplier;                            // �ۼ���ɫ
		multiplier *= 0.7f;
		// �ı����
		ray.Origin = payload.WorldPosition + payload.WorldNormal + 0.0001f; //�ڻ��е��ϼ���һ��΢С��ƫ��
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);   // �������
	}
	return glm::vec4(color, 1.0f);
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closeSphereIndex = -1;                           
	float closestT = std::numeric_limits<float>::max();  // ����ľ���

	for (size_t i = 0;i < m_ActiveScene->Spheres.size();i++) {
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 Origin = ray.Origin - sphere.Position;      // �����������귴���ƶ����ߵ����            
		float radius = sphere.Radius;                                     // ��İ뾶
		//��ķ��̣�x^2+y^2+z^2=radius   ��������(0,0)
		//float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
		//float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
		//float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y + rayOrigin.z * rayOrigin.z -  radius * radius;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(Origin, ray.Direction);
		float c = glm::dot(Origin, Origin) - radius * radius;
		float delta = b * b - 4.0f * a * c;

		if (delta < 0.0f)
			continue;  // δ��������򣬼�����һ����
		float inPointT = (-b - glm::sqrt(delta)) / (2.0f * a);      // ���ľ���t
		if (inPointT > 0.0f && inPointT < closestT) {
			closestT = inPointT;
			closeSphereIndex = (int)i;
		}
	}

	if(closeSphereIndex == -1)
		return Miss(ray);       
	return ClosestHit(ray, closestT, closeSphereIndex);  // �������� 
}

HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;
	const Sphere& closeSphere = m_ActiveScene->Spheres[objectIndex];
	glm::vec3 Origin = ray.Origin - closeSphere.Position;
	payload.WorldPosition = Origin + hitDistance * ray.Direction;    // �������
	payload.WorldNormal = glm::normalize(payload.WorldPosition);     // ����
	payload.WorldPosition += closeSphere.Position;                   // ��������

	return payload;
}

HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
