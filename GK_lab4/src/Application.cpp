#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "utility.h"
#include "KeyMan.h"
#include "ResMan.h"
#include "Level.h"
#include "Model.h"
#include "constants.h"
#include "Text.h"

bool firstMouse = true;
Player *playerRef;
float lastX = 0, lastY = 0;
float sensitivity = 0.001f;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates range from bottom to top
	lastX = (float)xpos;
	lastY = (float)ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	playerRef->pyr.y -= xoffset;
	playerRef->pyr.x += yoffset;

	if (playerRef->pyr.x > glm::radians(90.0f))
		playerRef->pyr.x = glm::radians(90.0f);
	else if(playerRef->pyr.x < -glm::radians(90.0f))
		playerRef->pyr.x = -glm::radians(90.0f);
}

bool throwHeld = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
			throwHeld = true;
		else
			throwHeld = false;
	}
}

// do przyciskow myszki jest oddzielny callback, mi bedzie potrzebny tylko do sprawdzania czy LMB wcisniete,
// wtedy rzucic obiektem jesli jakis trzymamy, po prostu tutaj zrobic funkcje

int main(void)
{
	GLFWwindow *window = nullptr;
	unsigned int width, height;
	if (util::initWindow("GK_lab4", &window, &width, &height) == -1)
		return -1;

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	unsigned int progVNShader = util::getShaderProgramFromName("VN");
	unsigned int progVTNShader = util::getShaderProgramFromName("VTN");
	unsigned int progInterfaceShader = util::getShaderProgramFromName("interface");
	unsigned int progCubeShader = util::getShaderProgramFromName("skybox");
	unsigned int progColShader = util::getShaderProgramFromName("col");

	srand((unsigned int)time(NULL));
	ResMan resMan;
	Player player(progVNShader, progColShader);
	playerRef = &player;
	Text textDrawer(progInterfaceShader, width, height);

	unsigned int currentLevelIndex = 0;
	Level currentLevel(resMan.levelNames[0], progVNShader, progVTNShader, progCubeShader);

	std::vector<Model *> models;
	for (unsigned int i = 0; i < resMan.modelNames.size(); i++)
		models.push_back(new Model(resMan.modelNames[i], progVNShader, progVTNShader, progColShader));

	currentLevel.passFoundModels(models);

	player.pyr.y = currentLevel.initialPlayerYaw;
	player.pos = currentLevel.initialPlayerPos;

	glm::vec3 overviewCamPos(0.0f, cnst::initOverviewCamDistance, 0.0f);
	bool overviewCamOverSphere = true, overviewCamRoll = false;
	glm::vec3 overviewCamPYR(glm::radians(-90.0f), 0.0f, 0.0f);
	float overviewCamDist = cnst::initOverviewCamDistance;

	float fovY = 45.0f;
	glm::vec3 lockedPos(player.pos.x, player.pos.y + cnst::playerHeight, player.pos.z);
	glm::vec3 lockedDirPos(player.pos.x, player.pos.y + cnst::playerHeight, player.pos.z);
	glm::vec3 lockedPY = player.pyr;
	bool blinnLighting = false;
	bool gouraudShading = false;
	bool wireframeDrawing = false;
	bool drawCollisions = false;
	bool displayPlayerLights = true;
	bool tookScreenshot = false;
	bool pressedRestart = false;
	KeyMan keyMan(window,
		&player,
		&width,
		&height,
		&tookScreenshot,
		&fovY,
		&lockedPos,
		&lockedPY,
		&lockedDirPos,
		&blinnLighting,
		&gouraudShading,
		&wireframeDrawing,
		&drawCollisions,
		&displayPlayerLights,
		&overviewCamOverSphere,
		&overviewCamRoll,
		&overviewCamDist,
		&sensitivity,
		&pressedRestart);
	keyMan.setInputMode(2); // 0 - rozgrywka, 1 - tylko spacja i esc, 2 - dev (testowe, tworzenie poziomow)

	char textFPS[] = "000FPS";
	char textExperiment[] = "EXPERIMENT 000";
	char textCam[] = "CAM000";
	char textProgress[] = "PROGRESS 000/000";
	unsigned int objectsCountCopy = currentLevel.objectsCollected.size();
	textProgress[15] = objectsCountCopy % 10 + '0';
	objectsCountCopy /= 10;
	textProgress[14] = objectsCountCopy % 10 + '0';
	objectsCountCopy /= 10;
	textProgress[13] = objectsCountCopy % 10 + '0';
	char textGraphicToggles[] = "  PP";

	glEnable(GL_DEPTH_TEST);
	glClearColor(cnst::clearColor.r, cnst::clearColor.g, cnst::clearColor.b, 0.0F);
	float cumulTime = 0.0f;
	int frameCount = 0;
	float deltaTime, currentFrame, lastFrame = 0.0f;
	bool firstFrame = true, levelCleared = false;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window) && !levelCleared)
	{
		currentFrame = (float)glfwGetTime();
		if (firstFrame || tookScreenshot)
		{
			// uproszczona fizyka, zbyt duze deltaTime ja psuje
			firstFrame = false;
			tookScreenshot = false;
			lastFrame = currentFrame;
		}
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		keyMan.processInput(deltaTime);

		frameCount++;
		cumulTime += deltaTime;
		if (cumulTime >= 1.0f)
		{
			std::cout << frameCount << std::endl;
			textFPS[2] = frameCount % 10 + '0';
			frameCount /= 10;
			textFPS[1] = frameCount % 10 + '0';
			frameCount /= 10;
			textFPS[0] = frameCount % 10 + '0';
			frameCount = 0;
			cumulTime -= 1.0f;
		}

		if(wireframeDrawing)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unsigned int mainWidth = width;
		unsigned int mainHeight = (unsigned int)(height * cnst::screenMainPortionHeight);
		glm::vec3 camPos = glm::vec3(player.pos.x, player.pos.y + cnst::playerHeight, player.pos.z);

		// aktualizujemy dane
		{
			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->updateRotation(deltaTime);

			// fizyka
			currentLevel.collidePlayer(&player, deltaTime);
			currentLevel.collideObjects(deltaTime);
			player.collideSphere();

			currentLevel.collectObject(&player);
			if (currentLevel.collectedCount == currentLevel.objectsCollected.size())
				levelCleared = true;

			if(!player.holdingObject)
				player.grabObject(models);
			else
			{
				glm::mat4 heldRotate = glm::rotate(glm::mat4(1.0f), player.pyr.y, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::vec3 heldOffset = heldRotate * glm::vec4(0.0f, 0.0f, -2.0f, 0.0f);
				glm::vec3 heldPos = currentLevel.collideHeld(player.pos + heldOffset);
				player.heldObjectModel->positions[player.heldObjectIndex] = heldPos;

				if (throwHeld)
				{
					player.holdingObject = false;
					glm::mat4 throwRotate = glm::rotate(glm::mat4(1.0f), player.pyr.x, glm::vec3(1.0f, 0.0f, 0.0f));
					glm::vec3 rotVel = heldRotate * throwRotate * glm::vec4(0.0f, 0.0f, -1.0f * cnst::throwVel, 0.0f);
					player.heldObjectModel->velocities[player.heldObjectIndex] = rotVel;
				}
			}

			// czy obiekt albo gracz spadli z planszy
			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->checkPositions();
			if (player.pos.y <= cnst::barrierHeight || pressedRestart)
			{
				pressedRestart = false;

				player.pos = currentLevel.initialPlayerPos;
				player.pyr = glm::vec3(0.0f, currentLevel.initialPlayerYaw, 0.0f);
				player.vel = glm::vec3(0.0f);
				player.holdingObject = false;

				currentLevel.restartLevel();
			}

			if (overviewCamOverSphere)
			{
				overviewCamPos = glm::vec3(0.0f, overviewCamDist, 0.0f);
				overviewCamPYR.x = glm::radians(-90.0f);
			}
			else
			{
				overviewCamPYR.x = glm::radians(cnst::overviewCamPitch);
				overviewCamPYR.y -= cnst::baseTestSpeed2 * deltaTime * 0.3f;

				overviewCamPos = glm::vec3(0.0f, 0.0f, overviewCamDist);
				glm::mat4 overviewCamRotate(1.0f);
				overviewCamRotate = glm::rotate(overviewCamRotate, overviewCamPYR.y, glm::vec3(0.0f, 1.0f, 0.0f));
				overviewCamRotate = glm::rotate(overviewCamRotate, overviewCamPYR.x, glm::vec3(1.0f, 0.0f, 0.0f));
				overviewCamPos = overviewCamRotate * glm::vec4(overviewCamPos, 0.0f);
			}

			if (overviewCamRoll)
				overviewCamPYR.z += cnst::baseTestSpeed2 * deltaTime;

			unsigned int restartCopy = currentLevel.restartCount;
			textExperiment[13] = restartCopy % 10 + '0';
			restartCopy /= 10;
			textExperiment[12] = restartCopy % 10 + '0';
			restartCopy /= 10;
			textExperiment[11] = restartCopy % 10 + '0';

			unsigned int collectedCountCopy = currentLevel.collectedCount;
			textProgress[11] = collectedCountCopy % 10 + '0';
			collectedCountCopy /= 10;
			textProgress[10] = collectedCountCopy % 10 + '0';
			collectedCountCopy /= 10;
			textProgress[9] = collectedCountCopy % 10 + '0';

			if (drawCollisions)
				textGraphicToggles[0] = 'C';
			else
				textGraphicToggles[0] = ' ';
			if (wireframeDrawing)
				textGraphicToggles[1] = 'W';
			else
				textGraphicToggles[1] = ' ';
			if (gouraudShading)
				textGraphicToggles[2] = 'G';
			else
				textGraphicToggles[2] = 'P';
			if (blinnLighting)
				textGraphicToggles[3] = 'B';
			else
				textGraphicToggles[3] = 'P';
		}

		// zbieramy wszystkie swiatla
		{
			currentLevel.resetLights();

			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->addLightsToGlobal(&currentLevel);

			if (displayPlayerLights)
			{
				glm::mat4 dirLightRotate(1.0f);
				dirLightRotate = glm::rotate(dirLightRotate, lockedPY.y, glm::vec3(0.0f, 1.0f, 0.0f));
				dirLightRotate = glm::rotate(dirLightRotate, lockedPY.x, glm::vec3(1.0f, 0.0f, 0.0f));
				glm::vec3 dirDir = glm::normalize(dirLightRotate * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

				std::vector<pntLight> myPnt;
				pntLight testPnt;
				testPnt.pos = lockedPos;
				testPnt.color = glm::vec3(1.0f, 0.0f, 0.0f);
				myPnt.push_back(testPnt);

				std::vector<dirLight> myDir;
				dirLight testDir;
				testDir.pos = lockedDirPos;
				testDir.dir = dirDir;
				testDir.focus = 32;
				testDir.color = glm::vec3(0.0f, 0.0f, 1.0f);
				myDir.push_back(testDir);

				currentLevel.addLights(myPnt, myDir);
			}

			// UWAGA - ten blad opisany ponizej najwyrazniej tez zalezy od wersji sterownikow/opengla
			// u mnie wszystkie swiatla dzialaja bez problemu, na wydzialowym pojawil sie ponizszy blad
			// (poza tym jedyna roznica w dzialaniu to mniejsza ilosc klatek na sekunde na moim laptopie,
			// spada ponizej 60, podczas gdy na wydzialowym byla stabilna - chyba nawet ograniczona jakimis systemowymi
			// ustawieniami do 60 - mozliwe ze sterowniki nvidii jakos agresywnie optymalizuja, ale raczej po prostu
			// karta zintegrowana jest wolniejsza od dedykowanej)
			//
			// dziwny blad ktorego nie umiem rozwiazac:
			// dla swiatla punktowego ktore jest dodawane do globalnej tablicy w pierwszej kolejnosci nie jest rysowany diffuse
			// dla ostatniego swiatla punktowego diffuse jest rysowany, ale jesli liczba swiatel punktowych sie zmniejszy
			// (gracz wylaczy swoje swiatlo, obiekt ze swiatlem punktowym zostanie zebrany)
			// to jego diffuse utyka w ostatnim miejscu w ktorym byl poprawnie narysowany
			// (nawet jesli swiatlo punktowe przestalo istniec, tzn. ostatnim swiatlem bylo swiatlo obiektu, ktory zostal zebrany)
			//
			// blad nie wystepuje dla: komponentu specular swiatel punktowych, swiatel kierunkowych, cieniowania w trybie Gouraud
			//
			// ostatecznie mozna zrobic kanapke ze swiatla poziomu, czyli dodawac je dwukrotnie - na poczatku i na koncu,
			// a zarazem umiescic je gdzies gdzie nie rzuca sie w oczy
		}

		// rysujemy glowny widok
		{
			glViewport(0, 0, mainWidth, mainHeight);
			glm::mat4 proj = glm::perspective(glm::radians(fovY), (float)mainWidth / mainHeight, 0.1f, 100.0f);
			glm::mat4 view = util::lookAtPYR(camPos, player.pyr);

			if (drawCollisions)
			{
				if (!wireframeDrawing)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				player.drawCol(view, proj);
				if (!wireframeDrawing)
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->draw(view, proj, camPos, blinnLighting, gouraudShading, wireframeDrawing, drawCollisions, &currentLevel);

			currentLevel.draw(view, proj, camPos, blinnLighting, gouraudShading);

			textDrawer.drawCursor(player.closeToWall, wireframeDrawing);
			textDrawer.drawText("DIRECT FEED", 0, 0, mainWidth, mainHeight, wireframeDrawing);
			textDrawer.drawText(cnst::instructions, mainWidth - 16 * 18, 16 * 11, mainWidth, mainHeight, wireframeDrawing);
			textDrawer.drawText(textFPS, mainWidth - 16 * 6, mainHeight - 16, mainWidth, mainHeight, wireframeDrawing);

		}

		// rysujemy najblizsza kamere sledzaca
		{
			glViewport(0, mainHeight, width / 2, height - mainHeight);
			glm::mat4 proj = glm::perspective(glm::radians(fovY), (float)(width / 2) / (height - mainHeight), 0.1f, 100.0f);

			unsigned int closestCamIndex = 0;
			float closestCamDist = glm::distance(camPos, currentLevel.trackingCams[0]);
			for (unsigned int i = 1; i < currentLevel.trackingCams.size(); i++)
			{
				if (glm::distance(camPos, currentLevel.trackingCams[i]) < closestCamDist)
				{
					closestCamDist = glm::distance(camPos, currentLevel.trackingCams[i]);
					closestCamIndex = i;
				}
			}
			glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::mat4 view = util::myLookAt(currentLevel.trackingCams[closestCamIndex], camPos, camUp, true);

			// z tego widoku rysujemy model gracza
			player.draw(view, proj, currentLevel.trackingCams[closestCamIndex], blinnLighting, gouraudShading, &currentLevel);

			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->draw(view, proj, currentLevel.trackingCams[closestCamIndex],
					blinnLighting, gouraudShading, wireframeDrawing, drawCollisions, &currentLevel);

			currentLevel.draw(view, proj, currentLevel.trackingCams[closestCamIndex], blinnLighting, gouraudShading);

			textCam[5] = closestCamIndex % 10 + '0';
			closestCamIndex /= 10;
			textCam[4] = closestCamIndex % 10 + '0';
			closestCamIndex /= 10;
			textCam[3] = closestCamIndex % 10 + '0';

			textDrawer.drawText(textCam, 0, height - mainHeight - 16, width / 2, height - mainHeight, wireframeDrawing);
			textDrawer.drawText(textExperiment, 0, 0, width / 2, height - mainHeight, wireframeDrawing);
		}

		// rysujemy kamere overview
		{
			glViewport(width / 2, mainHeight, width / 2, height - mainHeight);
			glm::mat4 proj = glm::perspective(glm::radians(fovY), (float)(width / 2) / (height - mainHeight), 0.1f, 100.0f);
			glm::mat4 view = util::lookAtPYR(overviewCamPos, overviewCamPYR);

			// z tego widoku rysujemy model gracza
			player.draw(view, proj, overviewCamPos, blinnLighting, gouraudShading, &currentLevel);

			for (unsigned int i = 0; i < models.size(); i++)
				models[i]->draw(view, proj, overviewCamPos,
					blinnLighting, gouraudShading, wireframeDrawing, drawCollisions, &currentLevel);

			currentLevel.draw(view, proj, overviewCamPos, blinnLighting, gouraudShading);

			textDrawer.drawText(textGraphicToggles, 0, height - mainHeight - 16, width / 2, height - mainHeight, wireframeDrawing);
			textDrawer.drawText(textProgress, 0, 0, width / 2, height - mainHeight, wireframeDrawing);
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	for (unsigned int i = 0; i < models.size(); i++)
		delete models[i];

	glfwTerminate();
	return 0;
}