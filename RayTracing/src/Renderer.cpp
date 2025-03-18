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
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVAerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;
	// 确保场景有BVH加速结构
	if (!m_ActiveScene->BVHRoot)
	{
		std::cout << "警告：场景没有BVH加速结构，正在构建..." << std::endl;
		const_cast<Scene*>(m_ActiveScene)->BuildBVH();
		std::cout << "BVH构建完成！" << std::endl;
	}
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
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
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

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 accumulatedLight{ 0.0f };
	glm::vec3 throughput{ 1.0f };
	uint32_t seed = x + y * m_FinalImage->GetWidth();  //俄罗斯轮盘赌
	const int maxBounces = 5;

	// 第一次是源光线，后面是反射的光线
	for (int bounce = 0; bounce < maxBounces; bounce++) {
		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			//glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			//accumulatedLight += skyColor * throughput;
			break;
		}
		if (payload.MaterialIndex < 0 || payload.MaterialIndex >= m_ActiveScene->Materials.size())
		{
			break; // 无效材质索引保护
		}
		const Material& material = m_ActiveScene->Materials[payload.MaterialIndex];

		accumulatedLight += material.GetEmission() * throughput;

		// 处理漫反射
		throughput *= material.Albedo;

		// 处理镜面反射
		glm::vec3 reflectedDirection = glm::reflect(ray.Direction, payload.WorldNormal);
		glm::vec3 specular = material.SpecularColor * glm::pow(glm::max(glm::dot(payload.WorldNormal, reflectedDirection), 0.0f), material.Shininess);
		accumulatedLight += specular * throughput;

		// 处理透射
		if (material.TransmissionColor != glm::vec3(1.0f)) {
			float eta = 1.0f / material.RefractionIndex;
			glm::vec3 refractedDirection = glm::refract(ray.Direction, payload.WorldNormal, eta);
			ray.Direction = glm::normalize(refractedDirection);
			throughput *= material.TransmissionColor;
		}
		else {
			// 改变光线方向
			ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
		}

		// 改变光线起点
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f; //在击中点上加上一个微小的偏移

		// 俄罗斯轮盘赌
		if (bounce > 3) {
			float luminance = glm::dot(throughput, glm::vec3(0.2126f, 0.7152f, 0.0722f));

			// 如果贡献度太小，直接终止
			const float threshold = 0.001f;
			if (luminance < threshold) {
				break;
			}

			// 根据亮度自适应调整概率
			float continueProbability = glm::clamp(luminance, 0.1f, 0.95f);

			if (Utils::RandomFloat(seed) > continueProbability) {
				break;
			}

			throughput /= continueProbability;
		}
	}
	return glm::vec4(accumulatedLight, 1.0f);
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	// 如果场景有BVH加速结构
	if (m_ActiveScene && m_ActiveScene->BVHRoot)
	{
		return TraceRayBVH(ray, m_ActiveScene->BVHRoot.get());
	}

	// 否则使用暴力方法
	return TraceRayBrute(ray);
}

// 原来的暴力求交方法
HitPayload Renderer::TraceRayBrute(const Ray& ray)
{
	int closestObjIndex = -1;
	float closestT = std::numeric_limits<float>::max();

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

	if (closestObjIndex == -1)
		return Miss(ray);

	return ClosestTriangleHit(ray, closestT, closestObjIndex);
}

HitPayload Renderer::ClosestTriangleHit(const Ray& ray, float hitDistance, int meshIndex) {
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = meshIndex;

	const Mesh& mesh = m_ActiveScene->Meshes[meshIndex];
	payload.MaterialIndex = mesh.MaterialIndex;
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

// BVH主入口
HitPayload Renderer::TraceRayBVH(const Ray& ray, const BVHNode* node)
{
	HitPayload payload;
	payload.HitDistance = std::numeric_limits<float>::max();

	// 使用栈进行非递归遍历
	std::stack<BVHTraversalInfo> nodeStack;
	const BVHNode* currentNode = node;
	float tMin = 0.0f, tMax = std::numeric_limits<float>::max();

	while (currentNode || !nodeStack.empty())
	{
		if (currentNode)
		{
			// 检查当前节点的包围盒是否与光线相交
			if (IntersectBVH(ray, currentNode, tMin, tMax))
			{
				if (currentNode->isLeaf)
				{
					// 叶子节点，检查所有三角形
					HitPayload leafHit = FindClosestHit(ray, currentNode);
					if (leafHit.HitDistance < payload.HitDistance)
					{
						payload = leafHit;
					}
					// 处理完叶子节点后从栈中取下一个节点
					if (!nodeStack.empty())
					{
						currentNode = nodeStack.top().node;
						tMin = nodeStack.top().tMin;
						nodeStack.pop();
					}
					else
					{
						currentNode = nullptr;
					}
				}
				else
				{
					// 内部节点，先处理较近的子节点
					float leftTMin, leftTMax, rightTMin, rightTMax;
					bool hitLeft = IntersectBVH(ray, currentNode->left.get(), leftTMin, leftTMax);
					bool hitRight = IntersectBVH(ray, currentNode->right.get(), rightTMin, rightTMax);

					if (hitLeft && hitRight)
					{
						// 两个子节点都相交，先处理较近的
						if (leftTMin < rightTMin)
						{
							nodeStack.push({ currentNode->right.get(), rightTMin });
							currentNode = currentNode->left.get();
							tMin = leftTMin;
						}
						else
						{
							nodeStack.push({ currentNode->left.get(), leftTMin });
							currentNode = currentNode->right.get();
							tMin = rightTMin;
						}
					}
					else if (hitLeft)
					{
						currentNode = currentNode->left.get();
						tMin = leftTMin;
					}
					else if (hitRight)
					{
						currentNode = currentNode->right.get();
						tMin = rightTMin;
					}
					else
					{
						if (!nodeStack.empty())
						{
							currentNode = nodeStack.top().node;
							tMin = nodeStack.top().tMin;
							nodeStack.pop();
						}
						else
						{
							currentNode = nullptr;
						}
					}
				}
			}
			else
			{
				// 当前节点不相交，从栈中取下一个节点
				if (!nodeStack.empty())
				{
					currentNode = nodeStack.top().node;
					tMin = nodeStack.top().tMin;
					nodeStack.pop();
				}
				else
				{
					currentNode = nullptr;
				}
			}
		}
	}

	return payload.HitDistance == std::numeric_limits<float>::max() ? Miss(ray) : payload;
}

// 检查光线是否与BVH节点的包围盒相交
bool Renderer::IntersectBVH(const Ray& ray, const BVHNode* node, float& tMin, float& tMax)
{
	if (!node) return false;
	return node->bounds.IntersectRay(ray, tMin, tMax);
}

// 在叶子节点中查找最近的交点
HitPayload Renderer::FindClosestHit(const Ray& ray, const BVHNode* node)
{
	HitPayload payload;
	payload.HitDistance = std::numeric_limits<float>::max();

	// 遍历叶子节点中的所有三角形
	for (size_t triIdx : node->triangleIndices)
	{
		// 遍历所有网格找到对应的三角形
		for (const auto& mesh : m_ActiveScene->Meshes)
		{
			if (triIdx < mesh.Triangles.size())
			{
				const Triangle& triangle = mesh.Triangles[triIdx];
				float t;
				if (RayTriangleIntersect(ray, triangle, t))
				{
					if (t > 0 && t < payload.HitDistance)
					{
						payload.HitDistance = t;
						payload.WorldPosition = ray.Origin + ray.Direction * t;

						// 计算法线
						glm::vec3 edge1 = triangle.V1.Position - triangle.V0.Position;
						glm::vec3 edge2 = triangle.V2.Position - triangle.V0.Position;
						payload.WorldNormal = glm::normalize(glm::cross(edge1, edge2));

						// 设置材质索引
						payload.MaterialIndex = triangle.MaterialIndex;
					}
				}
			}
		}
	}
	return payload;
}
