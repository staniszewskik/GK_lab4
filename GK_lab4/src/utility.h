#pragma once

#include <glm/glm.hpp>

#include "Level.h"

namespace util
{
	bool readObj(const char *path, float **vertices, unsigned int *vertSize, unsigned int **indices, unsigned int *indcSize);
	void readLights(std::string name, std::vector<pntLight> *pnts, std::vector<dirLight> *dirs, bool level,
		glm::vec3 *ambient, sunLight *sun);

	void bindVAO_VN(unsigned int *VAO, unsigned int *VBO, unsigned int *EBO,
		const float vertices[], unsigned int vertSize, const unsigned int indices[], unsigned int indcSize);
	void bindVAO_VTN(unsigned int *VAO, unsigned int *VBO, unsigned int *EBO,
		const float vertices[], unsigned int vertSize, const unsigned int indices[], unsigned int indcSize);

	glm::mat4 myLookAt(glm::vec3 pos, glm::vec3 tgt, glm::vec3 up, bool fallbackRight);
	glm::mat4 lookAtPYR(glm::vec3 pos, glm::vec3 pyr);

	std::string readFile(const char *fileName);
	unsigned int createShader(const char *path, GLenum shaderType);
	unsigned int buildShaderProgram(unsigned int vert, unsigned int frag);
	unsigned int getShaderProgramFromName(std::string name);
	unsigned int createBasicTexture(const char *path);
	unsigned int createCubemap(std::vector<std::string> faces);
	void bindVAO_Cubemap(unsigned int *VAO, unsigned int *VBO);
	int initWindow(const char *title, GLFWwindow **window, unsigned int *width, unsigned int *height);

	unsigned int createInterfaceTexture();
	void bindVAO_Text(unsigned int *VAO, unsigned int *VBO, const float vertices[], unsigned int vertSize);
}