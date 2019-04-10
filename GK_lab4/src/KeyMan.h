#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Player.h"

class KeyMan
{
public:
	KeyMan(GLFWwindow *window,
		Player *playerRef,
		unsigned int *width,
		unsigned int *height,
		bool *tookScreenshot,
		float *fovY,
		glm::vec3 *lockedPos,
		glm::vec3 *lockedPY,
		glm::vec3 *lockedDirPos,
		bool *blinnLighting,
		bool *gouraudShading,
		bool *wireframeDrawing,
		bool *drawCollisions,
		bool *displayPlayerLights,
		bool *overviewCamOverSphere,
		bool *overviewCamRoll,
		float *overviewCamDist,
		float *sensitivity,
		bool *pressedRestart)
	{
		this->window = window;
		this->playerRef = playerRef;
		this->width = width;
		this->height = height;
		this->tookScreenshot = tookScreenshot;

		this->fovY = fovY;
		this->lockedPos = lockedPos; lockedPosLastState = false; lockedPosOn = false;
		this->lockedPY = lockedPY; lockedYawLastState = false; lockedYawOn = false;
		this->lockedDirPos = lockedDirPos;
		this->blinnLighting = blinnLighting; blinnLightingLastState = false;
		this->gouraudShading = gouraudShading; gouraudShadingLastState = false;
		this->wireframeDrawing = wireframeDrawing; wireframeDrawingLastState = false;
		this->drawCollisions = drawCollisions; wireframeDrawingLastState = false;
		this->displayPlayerLights = displayPlayerLights; displayPlayerLightsLastState = false;
		this->overviewCamOverSphere = overviewCamOverSphere; overviewCamOverSphereLastState = false;
		this->overviewCamRoll = overviewCamRoll; overviewCamRollLastState = false;
		this->overviewCamDist = overviewCamDist;
		this->sensitivity = sensitivity;
		this->pressedRestart = pressedRestart; pressedRestartLastState = false;

		spaceLastState = false;
		printScreenLastState = false;
	}

	void processInput(float deltaTime);
	void setInputMode(unsigned int mode);
	unsigned int getInputMode();
private:
	GLFWwindow *window;
	unsigned int inputMode;
	Player *playerRef;
	unsigned int *width;
	unsigned int *height;
	bool *tookScreenshot;

	float *fovY;
	glm::vec3 *lockedPos; bool lockedPosLastState; bool lockedPosOn;
	glm::vec3 *lockedPY; bool lockedYawLastState; bool lockedYawOn;
	glm::vec3 *lockedDirPos;
	bool *blinnLighting; bool blinnLightingLastState;
	bool *gouraudShading; bool gouraudShadingLastState;
	bool *wireframeDrawing; bool wireframeDrawingLastState;
	bool *drawCollisions; bool drawCollisionsLastState;
	bool *displayPlayerLights; bool displayPlayerLightsLastState;
	bool *overviewCamOverSphere; bool overviewCamOverSphereLastState;
	bool *overviewCamRoll; bool overviewCamRollLastState;
	float *overviewCamDist;
	float *sensitivity;
	bool *pressedRestart; bool pressedRestartLastState;

	bool spaceLastState;
	bool printScreenLastState;

	void processSpaceEsc();
	void processWSAD(float deltaTime);
	void processGraphicToggles();
	void processTest(float deltaTime);
};

