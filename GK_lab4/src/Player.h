#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Model;
class Level;

class Player
{
public:
	Player(unsigned int progVN, unsigned int progCol);
	~Player();
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS, Level *l);
	void drawCol(glm::mat4 view, glm::mat4 proj);
	void grabObject(std::vector<Model *> models);
	void collideSphere();

	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 pyr;
	glm::vec3 dir;

	bool onGround;
	bool closeToWall;

	// indeks w wektorze pozycji na ktorych jest model do ktorego mamy wskaznik
	unsigned int heldObjectIndex;
	Model *heldObjectModel;
	bool holdingObject;
private:
	unsigned int VAO, VBO, EBO;
	float *vert;
	unsigned int *indc;
	unsigned int vertSize, indcSize;

	unsigned int progVNShader;
	unsigned int progColShader;

	unsigned int colVAO, colVBO, colEBO;
	float *colVert;
	unsigned int *colIndc;
	unsigned int colVertSize, colIndcSize;
};

