#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Sphere
{
	glm::vec3 Position;
	float Radius;
	glm::vec3 Color;
};

struct Scene
{
	std::vector<Sphere> Spheres;
};