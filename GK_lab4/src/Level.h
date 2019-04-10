#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

#include "constants.h"
#include "helperStructs.h"
#include "Player.h"

class Model;
class Player;

class Level
{
public:
	Level(std::string name, unsigned int progVN, unsigned int progVTN, unsigned int progCube);
	~Level();
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS);
	bool addLights(std::vector<pntLight> pnts, std::vector<dirLight> dirs);
	void resetLights();
	void passFoundModels(std::vector<Model *> found);
	unsigned int addObject(glm::vec3 pos);
	void modifyObject(glm::vec3 pos, unsigned int i);
	void removeObject(unsigned int i);
	unsigned int addPlatform(glm::vec3 pos, unsigned int type);
	void modifyPlatform(platform newVals, unsigned int type, unsigned int i);
	void removePlatform(unsigned int type, unsigned int i);
	void restartLevel();
	void saveLevel();

	void collidePlayer(Player *p, float deltaTime);
	void collideObjects(float deltaTime);
	glm::vec3 collideHeld(glm::vec3 pos);
	void collectObject(Player *p);

	glm::vec3 ambient;
	sunLight sun;
	unsigned int globalPntsCount;
	pntLight globalPnts[cnst::maxLights]; // przed kazdym rysowaniem przepisujemy swiatla z poziomu i wszystkich modeli,
	unsigned int globalDirsCount;
	dirLight globalDirs[cnst::maxLights]; // tyle ile sie zmiesci

	unsigned int restartCount;

	glm::vec3 initialPlayerPos;
	float initialPlayerYaw;

	std::vector<glm::vec3> trackingCams;

	unsigned int collectedCount;
	std::vector<bool> objectsCollected;
private:
	std::vector<pntLight> levelPnts;
	std::vector<dirLight> levelDirs;

	std::vector<glm::vec3> initialObjectPositions;
	std::vector<Model *> models;

	// platformy nie maja indeksow bo sa generowane w programie
	std::vector<platform> platforms[3];
	unsigned int platformVAO[3], platformVBO[3];
	float *platformVert[3];
	unsigned int platformVertSize[3];
	unsigned int platformTextureID[3];

	unsigned int sphereVAO, sphereVBO, sphereEBO;
	float *sphereVert;
	unsigned int *sphereIndc;
	unsigned int sphereVertSize, sphereIndcSize;

	unsigned int cubeVAO, cubeVBO, cubeTextureID;

	unsigned int progVNShader;
	unsigned int progVTNShader;
	unsigned int progCubeShader;

	bool collideSingle(glm::vec3 pos, float r, float h, platform plt, glm::vec3 *cor, bool *closeToWall);
	void genPlatforms(unsigned int type);
	void distributeModelPositions();
};

