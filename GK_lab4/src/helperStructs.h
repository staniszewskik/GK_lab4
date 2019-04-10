#pragma once

#include <glm/glm.hpp>

struct sunLight
{
	glm::vec3 dir;
	glm::vec3 color;
};

struct pntLight
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct dirLight
{
	glm::vec3 pos;
	glm::vec3 dir;
	int focus;
	glm::vec3 color;
};

struct platform
{
	glm::vec3 pos;
	glm::vec3 dim;
};