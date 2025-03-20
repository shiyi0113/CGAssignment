#include "Scene.h"
#include "Renderer.h"

bool AABB::IntersectRay(const Ray& ray, float& tMin, float& tMax) const
{
    glm::vec3 invDir = 1.0f / ray.Direction;
    glm::vec3 t0 = (min - ray.Origin) * invDir;
    glm::vec3 t1 = (max - ray.Origin) * invDir;

    glm::vec3 tSmaller = glm::min(t0, t1);
    glm::vec3 tBigger = glm::max(t0, t1);

    tMin = glm::max(tSmaller.x, glm::max(tSmaller.y, tSmaller.z));
    tMax = glm::min(tBigger.x, glm::min(tBigger.y, tBigger.z));

    return tMax >= tMin && tMax > 0;
}

void Mesh::UpdateBounds()
{
    bounds = AABB();
    for (const auto& triangle : Triangles) {
        bounds.Expand(triangle.GetBounds());
    }
}

void Scene::BuildBVH()
{
    if (Meshes.empty()) return;

    // 更新所有Mesh的包围盒
    for (auto& mesh : Meshes) {
        mesh.UpdateBounds();
    }

    // 收集所有三角形信息
    std::vector<std::pair<size_t, size_t>> allTriangles; // pair<mesh索引, 三角形索引>
    for (size_t meshIdx = 0; meshIdx < Meshes.size(); ++meshIdx) {
        for (size_t triIdx = 0; triIdx < Meshes[meshIdx].Triangles.size(); ++triIdx) {
            allTriangles.push_back({ meshIdx, triIdx });
        }
    }

    // 构建BVH树
    BVHRoot = BuildBVHNode(allTriangles, 0, allTriangles.size());
}

std::unique_ptr<BVHNode> Scene::BuildBVHNode(std::vector<std::pair<size_t, size_t>>& triangles, size_t start, size_t end)
{
    auto node = std::make_unique<BVHNode>();

    // 计算节点包围盒
    for (size_t i = start; i < end; i++) {
        const auto& [meshIdx, triIdx] = triangles[i];
        node->bounds.Expand(Meshes[meshIdx].Triangles[triIdx].GetBounds());
    }

    size_t triCount = end - start;

    // 如果三角形数量小于阈值，创建叶子节点
    if (triCount <= 4) {
        node->isLeaf = true;
        for (size_t i = start; i < end; i++) {
            const auto& [meshIdx, triIdx] = triangles[i];
            node->triangleIndices.push_back(triIdx);
        }
        return node;
    }

    // 选择分割轴（使用最长轴）
    glm::vec3 extent = node->bounds.max - node->bounds.min;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;

    // 根据选定轴对三角形进行排序
    size_t mid = (start + end) / 2;
    std::nth_element(triangles.begin() + start,
        triangles.begin() + mid,
        triangles.begin() + end,
        [this, axis](const auto& a, const auto& b) {
            const auto& triA = Meshes[a.first].Triangles[a.second];
            const auto& triB = Meshes[b.first].Triangles[b.second];
            return triA.GetCentroid()[axis] < triB.GetCentroid()[axis];
        });

    // 递归构建子树
    node->left = BuildBVHNode(triangles, start, mid);
    node->right = BuildBVHNode(triangles, mid, end);

    return node;
}
void Scene::Clear()
{
    // 清空所有网格
    for (auto& mesh : Meshes)
    {
        mesh.Triangles.clear();
    }
    Meshes.clear();

    // 清空所有材质
    for (auto& material : Materials)
    {
        material.DiffuseTextureData.clear();
    }
    Materials.clear();

    // 重置BVH根节点
    BVHRoot.reset();
}