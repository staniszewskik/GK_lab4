#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#include "helperStructs.h"

class Level;
class Player;

class Model
{
public:
	Model(std::string name, unsigned int progVN, unsigned int progVTN, unsigned int progCol);
	~Model();
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS, bool wF, bool dC, Level *l);
	void updateRotation(float deltaTime);
	bool addLightsToGlobal(Level *l);
	void addPosition(glm::vec3 pos, unsigned int i);
	void clearPositions();
	unsigned int removePosition(unsigned int i);
	void checkPositions();

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> initialPositions;
	std::vector<glm::vec3> velocities;
	std::vector<unsigned int> posIndex;
private:
	bool textured;
	unsigned int VAO, VBO, EBO;
	float *vert;
	unsigned int *indc;
	unsigned int vertSize, indcSize;
	unsigned int textureID;
	glm::vec3 untexturedColor;

	float rotation;
	float scale;
	float bottom;
	std::vector<pntLight> modelPnts;
	std::vector<dirLight> modelDirs;

	unsigned int progShaderID;
	unsigned int progColShader;

	unsigned int colVAO, colVBO, colEBO;
	float *colVert;
	unsigned int *colIndc;
	unsigned int colVertSize, colIndcSize;

	void findScaleAndBottom();
};

