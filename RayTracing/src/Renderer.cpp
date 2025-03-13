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

	// 重置进度计数器
	m_PixelsCompleted = 0;
	const uint32_t totalPixels = m_FinalImage->GetWidth() * m_FinalImage->GetHeight();

#define Multithreading 1
#if Multithreading
	std::for_each(std::execution::par, m_ImageVAerticalIter.begin(), m_ImageVAerticalIter.end(),
		[&](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y, totalPixels](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);                             // 返回该像素的颜色
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color; // 累加颜色
					glm::vec4 accumlatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumlatedColor /= (float)m_FrameIndex;  // 平均颜色,累加几次除以几，防止过亮
					accumlatedColor = glm::clamp(accumlatedColor, glm::vec4(0.0f), glm::vec4(1.0f));  // 将颜色的值限制在[0,1]内
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumlatedColor);

					// 更新进度并打印
					uint32_t completed = ++m_PixelsCompleted;
					if (completed % (totalPixels / 100) == 0) // 每完成1%打印一次
					{
						float progress = (float)completed / totalPixels * 100.0f;
						printf("\r渲染进度: %.1f%%", progress);
						fflush(stdout);
					}
				});
		});
	printf("\n"); // 完成后换行
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

	glm::vec3 accumulatedLight{ 0.0f };
	glm::vec3 throughput{ 1.0f };
	uint32_t seed = x + y * m_FinalImage->GetWidth();
	const int maxBounces = 5;
	// 第一次是源光线，后面是反射的光线
	for (int bounce = 0; bounce < maxBounces; bounce++) {
		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			accumulatedLight += skyColor * throughput;
			break;
		}
		if (payload.MaterialIndex < 0 || payload.MaterialIndex >= m_ActiveScene->Materials.size())
		{
			break; // 无效材质索引保护
		}
		const Material& material = m_ActiveScene->Materials[payload.MaterialIndex];
		accumulatedLight += material.GetEmission() * throughput;
		throughput *= material.Albedo;
		// 改变光线
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f; //在击中点上加上一个微小的偏移
		//ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));   // 反射光线+粗糙度
		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
		// 俄罗斯轮盘赌终止
		if (bounce > 3) {
			float continueProbability = glm::max(throughput.r, glm::max(throughput.g, throughput.b));
			if (Utils::RandomFloat(seed) >= continueProbability)
				break;
			throughput /= continueProbability;
		}
	}
	return glm::vec4(glm::clamp(accumulatedLight, 0.0f, 1.0f), 1.0f);
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestObjIndex = -1;
	float closestT = std::numeric_limits<float>::max();  // 最近的距离

	// 检查网格
	for (size_t meshIdx = 0; meshIdx < m_ActiveScene->Meshes.size(); ++meshIdx) {
		const auto& mesh = m_ActiveScene->Meshes[meshIdx];
		for (size_t triIdx = 0; triIdx < mesh.Triangles.size(); ++triIdx) {
			const auto& triangle = mesh.Triangles[triIdx];
			float t;
			if (RayTriangleIntersect(ray, triangle, t) && t < closestT) {
				closestT = t;
				closestObjIndex = meshIdx;
			}
		}
	}
	if(closestObjIndex == -1)
		return Miss(ray);       
	return ClosestTriangleHit(ray,closestT, closestObjIndex);  
}

HitPayload Renderer::ClosestTriangleHit(const Ray& ray, float hitDistance, int meshIndex) {
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = meshIndex;

	const Mesh& mesh = m_ActiveScene->Meshes[meshIndex];
	// 遍历所有三角形，找到与光线相交的三角形
	for (const auto& triangle : mesh.Triangles) {
		float t;
		if (RayTriangleIntersect(ray, triangle, t) && t == hitDistance) {
			// 计算交点坐标
			payload.WorldPosition = ray.Origin + t * ray.Direction;

			// 计算法线（可以使用顶点法线插值）
			glm::vec3 edge1 = triangle.V1.Position - triangle.V0.Position;
			glm::vec3 edge2 = triangle.V2.Position - triangle.V0.Position;
			payload.WorldNormal = glm::normalize(glm::cross(edge1, edge2));

			// 如果三角形有顶点法线，可以使用插值计算法线
			if (triangle.V0.Normal != glm::vec3(0.0f) && triangle.V1.Normal != glm::vec3(0.0f) && triangle.V2.Normal != glm::vec3(0.0f)) {
				glm::vec3 barycentricCoord = CalculateBarycentricCoord(payload.WorldPosition, triangle);
				payload.WorldNormal = glm::normalize(
					barycentricCoord.x * triangle.V0.Normal +
					barycentricCoord.y * triangle.V1.Normal +
					barycentricCoord.z * triangle.V2.Normal
				);
			}

			break;
		}
	}
	return payload;
}
HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
// Möller-Trumbore算法
bool Renderer::RayTriangleIntersect(const Ray& ray, const Triangle& triangle, float& t)
{
	const float EPSILON = 0.0000001f;
	glm::vec3 edge1 = triangle.V1.Position - triangle.V0.Position;
	glm::vec3 edge2 = triangle.V2.Position - triangle.V0.Position;

	glm::vec3 h = glm::cross(ray.Direction, edge2);
	float a = glm::dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return false;  // 射线与三角形平行

	float f = 1.0f / a;
	glm::vec3 s = ray.Origin - triangle.V0.Position;
	float u = f * glm::dot(s, h);

	if (u < 0.0 || u > 1.0)
		return false;

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(ray.Direction, q);

	if (v < 0.0 || u + v > 1.0)
		return false;

	t = f * glm::dot(edge2, q);
	return t > EPSILON;
}

glm::vec3 Renderer::CalculateBarycentricCoord(const glm::vec3& point, const Triangle& triangle) {
	glm::vec3 v0 = triangle.V1.Position - triangle.V0.Position;
	glm::vec3 v1 = triangle.V2.Position - triangle.V0.Position;
	glm::vec3 v2 = point - triangle.V0.Position;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return glm::vec3(u, v, w);
}