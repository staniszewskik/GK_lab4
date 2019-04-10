#include "Level.h"

#include <fstream>
#include <sstream>

#include "utility.h"
#include "Model.h"
#include "Player.h"

Level::Level(std::string name, unsigned int progVN, unsigned int progVTN, unsigned int progCube)
{
	std::ifstream in("resources/levels/" + name + ".levelgeo");
	std::string line = "";

	std::getline(in, line);
	while (line != "")
	{
		std::istringstream splitter(line);
		std::string lineType;
		splitter >> lineType;

		if (lineType == "pltfrm")
		{
			platform readPlatform;
			unsigned int readType;

			splitter >> readPlatform.pos.x >> readPlatform.pos.y >> readPlatform.pos.z
				>> readPlatform.dim.x >> readPlatform.dim.y >> readPlatform.dim.z
				>> readType;

			platforms[readType].push_back(readPlatform);
		}
		else if (lineType == "player")
		{
			splitter >> initialPlayerPos.x >> initialPlayerPos.y >> initialPlayerPos.z >> initialPlayerYaw;

			initialPlayerYaw = glm::radians(initialPlayerYaw);
		}
		else if (lineType == "object")
		{
			glm::vec3 readPos;
			splitter >> readPos.x >> readPos.y >> readPos.z;

			initialObjectPositions.push_back(readPos);
		}
		else if (lineType == "camera")
		{
			glm::vec3 readPos;
			splitter >> readPos.x >> readPos.y >> readPos.z;

			trackingCams.push_back(readPos);
		}

		std::getline(in, line);
	}

	util::readLights(name, &levelPnts, &levelDirs, true, &ambient, &sun);
	sun.dir = glm::normalize(sun.dir);
	for (unsigned int i = 0; i < levelDirs.size(); i++)
		levelDirs[i].dir = glm::normalize(levelDirs[i].dir);
	globalPntsCount = 0;
	globalDirsCount = 0;
	addLights(levelPnts, levelDirs);

	restartCount = 0;
	collectedCount = 0;
	for (unsigned int i = 0; i < initialObjectPositions.size(); i++)
		objectsCollected.push_back(false);

	progVNShader = progVN;
	progVTNShader = progVTN;
	progCubeShader = progCube;

	std::vector<std::string> faces
	{
		"resources/textures/" + name + "_" + cnst::skyboxOrder[0] + ".png",
		"resources/textures/" + name + "_" + cnst::skyboxOrder[1] + ".png",
		"resources/textures/" + name + "_" + cnst::skyboxOrder[2] + ".png",
		"resources/textures/" + name + "_" + cnst::skyboxOrder[3] + ".png",
		"resources/textures/" + name + "_" + cnst::skyboxOrder[4] + ".png",
		"resources/textures/" + name + "_" + cnst::skyboxOrder[5] + ".png"
	};
	cubeTextureID = util::createCubemap(faces);

	platformTextureID[0] = util::createBasicTexture(("resources/textures/" + name + "_0.png").c_str());
	platformTextureID[1] = util::createBasicTexture(("resources/textures/" + name + "_1.png").c_str());
	platformTextureID[2] = util::createBasicTexture(("resources/textures/" + name + "_2.png").c_str());
	
	util::readObj("resources/levels/sphere.obj", &sphereVert, &sphereVertSize, &sphereIndc, &sphereIndcSize);
	util::bindVAO_VN(&sphereVAO, &sphereVBO, &sphereEBO, sphereVert, sphereVertSize, sphereIndc, sphereIndcSize);

	util::bindVAO_Cubemap(&cubeVAO, &cubeVBO);

	// wypelniamy std::vector<platform> dla wszystkich trzech typow,
	// po wczytaniu poziomu wywolujemy dla nich genPlatforms
	for (int i = 0; i < 3; i++)
	{
		if (platforms[i].size() > 0)
			genPlatforms(i);
	}
}

Level::~Level()
{
	glDeleteTextures(3, platformTextureID);
	glDeleteTextures(1, &cubeTextureID);

	for (int i = 0; i < 3; i++)
		delete[] platformVert[i];
	glDeleteVertexArrays(3, platformVAO);
	glDeleteBuffers(3, platformVBO);

	delete[] sphereVert;
	delete[] sphereIndc;
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereEBO);

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
}

void Level::draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS)
{
	// sfera
	glUseProgram(progVNShader);

	glUniform3f(glGetUniformLocation(progVNShader, "sun.color"), sun.color.r, sun.color.g, sun.color.b);
	glUniform3f(glGetUniformLocation(progVNShader, "sun.dir"), sun.dir.x, sun.dir.y, sun.dir.z);

	glUniform3f(glGetUniformLocation(progVNShader, "ambient"), ambient.r, ambient.g, ambient.b);
	glUniform3f(glGetUniformLocation(progVNShader, "objectColor"), cnst::sphereColor.r, cnst::sphereColor.g, cnst::sphereColor.b);

	glUniform3f(glGetUniformLocation(progVNShader, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

	glUniform1i(glGetUniformLocation(progVNShader, "pntSize"), globalPntsCount);
	glUniform1i(glGetUniformLocation(progVNShader, "dirSize"), globalDirsCount);
	for (unsigned int i = 0; i < globalPntsCount; i++)
	{
		std::string pntName = "pnts[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVNShader, (pntName + ".pos").c_str()),
			globalPnts[i].pos.x, globalPnts[i].pos.y, globalPnts[i].pos.z);
		glUniform3f(glGetUniformLocation(progVNShader, (pntName + ".color").c_str()),
			globalPnts[i].color.r, globalPnts[i].color.g, globalPnts[i].color.b);
	}
	for (unsigned int i = 0; i < globalDirsCount; i++)
	{
		std::string dirName = "dirs[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".pos").c_str()),
			globalDirs[i].pos.x, globalDirs[i].pos.y, globalDirs[i].pos.z);
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".dir").c_str()),
			globalDirs[i].dir.x, globalDirs[i].dir.y, globalDirs[i].dir.z);
		glUniform1i(glGetUniformLocation(progVNShader, (dirName + ".focus").c_str()), globalDirs[i].focus);
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".color").c_str()),
			globalDirs[i].color.r, globalDirs[i].color.g, globalDirs[i].color.b);
	}

	glm::mat4 sphereMdel(1.0f);
	sphereMdel = glm::scale(sphereMdel, glm::vec3(cnst::sphereRadius, cnst::sphereRadius, cnst::sphereRadius));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "mdel"), 1, GL_FALSE, glm::value_ptr(sphereMdel));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glm::mat3 sphereNorm(glm::transpose(glm::inverse(sphereMdel)));
	glUniformMatrix3fv(glGetUniformLocation(progVNShader, "norm"), 1, GL_FALSE, glm::value_ptr(sphereNorm));

	glUniform1i(glGetUniformLocation(progVNShader, "blinnLighting"), bL);
	glUniform1i(glGetUniformLocation(progVNShader, "gouraudShading"), gS);

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sphereIndcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

	// platformy
	glUseProgram(progVTNShader);
	glUniform1i(glGetUniformLocation(progVTNShader, "ourTexture"), 1);

	glUniform3f(glGetUniformLocation(progVTNShader, "sun.color"), sun.color.r, sun.color.g, sun.color.b);
	glUniform3f(glGetUniformLocation(progVTNShader, "sun.dir"), sun.dir.x, sun.dir.y, sun.dir.z);

	glUniform3f(glGetUniformLocation(progVTNShader, "ambient"), ambient.r, ambient.g, ambient.b);
	glUniform1i(glGetUniformLocation(progVTNShader, "ourTexture"), 1);

	glUniform3f(glGetUniformLocation(progVTNShader, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

	glUniform1i(glGetUniformLocation(progVTNShader, "pntSize"), globalPntsCount);
	glUniform1i(glGetUniformLocation(progVTNShader, "dirSize"), globalDirsCount);
	for (unsigned int i = 0; i < globalPntsCount; i++)
	{
		std::string pntName = "pnts[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVTNShader, (pntName + ".pos").c_str()),
			globalPnts[i].pos.x, globalPnts[i].pos.y, globalPnts[i].pos.z);
		glUniform3f(glGetUniformLocation(progVTNShader, (pntName + ".color").c_str()),
			globalPnts[i].color.r, globalPnts[i].color.g, globalPnts[i].color.b);
	}
	for (unsigned int i = 0; i < globalDirsCount; i++)
	{
		std::string dirName = "dirs[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVTNShader, (dirName + ".pos").c_str()),
			globalDirs[i].pos.x, globalDirs[i].pos.y, globalDirs[i].pos.z);
		glUniform3f(glGetUniformLocation(progVTNShader, (dirName + ".dir").c_str()),
			globalDirs[i].dir.x, globalDirs[i].dir.y, globalDirs[i].dir.z);
		glUniform1i(glGetUniformLocation(progVTNShader, (dirName + ".focus").c_str()), globalDirs[i].focus);
		glUniform3f(glGetUniformLocation(progVTNShader, (dirName + ".color").c_str()),
			globalDirs[i].color.r, globalDirs[i].color.g, globalDirs[i].color.b);
	}

	glm::mat4 unitMdel(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(progVTNShader, "mdel"), 1, GL_FALSE, glm::value_ptr(unitMdel));
	glUniformMatrix4fv(glGetUniformLocation(progVTNShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(progVTNShader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glm::mat3 unitNorm(1.0f);
	glUniformMatrix3fv(glGetUniformLocation(progVTNShader, "norm"), 1, GL_FALSE, glm::value_ptr(unitNorm));

	glUniform1i(glGetUniformLocation(progVTNShader, "blinnLighting"), bL);
	glUniform1i(glGetUniformLocation(progVTNShader, "gouraudShading"), gS);

	for (int p = 0; p < 3; p++)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, platformTextureID[p]);

		glBindVertexArray(platformVAO[p]);
		glDrawArrays(GL_TRIANGLES, 0, platformVertSize[p] / (sizeof(float) * 8));
	}

	// skybox
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureID);

	glUseProgram(progCubeShader);
	glUniform1i(glGetUniformLocation(progCubeShader, "skybox"), 3);

	glm::mat4 nonmovingView = glm::mat4(glm::mat3(view));
	glUniformMatrix4fv(glGetUniformLocation(progCubeShader, "view"), 1, GL_FALSE, glm::value_ptr(nonmovingView));
	glUniformMatrix4fv(glGetUniformLocation(progCubeShader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(cubeVAO);
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS); // set depth function back to default
}

bool Level::addLights(std::vector<pntLight> pnts, std::vector<dirLight> dirs)
{
	unsigned int pntIndex = 0;
	while (pntIndex < pnts.size() && globalPntsCount < cnst::maxLights)
		globalPnts[globalPntsCount++] = pnts[pntIndex++];

	unsigned int dirIndex = 0;
	while (dirIndex < dirs.size() && globalDirsCount < cnst::maxLights)
		globalDirs[globalDirsCount++] = dirs[dirIndex++];

	// jesli da sie jeszcze jakies swiatla wstawic to zwracamy true
	// (moze ladnie by bylo zwracac ktore swiatla jeszcze da sie wstawic ale oj tam)
	return globalPntsCount < cnst::maxLights || globalDirsCount < cnst::maxLights;
}

void Level::resetLights()
{
	globalPntsCount = levelPnts.size();
	globalDirsCount = levelDirs.size();
}

void Level::passFoundModels(std::vector<Model *> found)
{
	for (unsigned int i = 0; i < found.size(); i++)
		models.push_back(found[i]);

	distributeModelPositions();
}

unsigned int Level::addObject(glm::vec3 pos)
{
	// dodaje do objectsCollected i initialObjectsPositions

	return 0;
}

void Level::modifyObject(glm::vec3 pos, unsigned int i)
{

}

void Level::removeObject(unsigned int i)
{

}

unsigned int Level::addPlatform(glm::vec3 pos, unsigned int type)
{
	// dodaje do odpowiedniego wektora platform nowa platforme, o domyslnym rozmiarze i na podanej pozycji
	platform toAdd;
	toAdd.pos = pos;
	toAdd.dim = glm::vec3(3.0f, 3.0f, 3.0f);
	platforms[type].push_back(toAdd);

	// zwalnia VAO, VBO i takie tam i tworzy na nowo

	return 0;
}

void Level::modifyPlatform(platform newVals, unsigned int type, unsigned int i)
{
	// modyfikuje odpowiednia platforme w odpowiednim wektorze
	// zwalnia VAO, VBO i takie tam i tworzy na nowo
}

void Level::removePlatform(unsigned int type, unsigned int i)
{
	// usuwa odpowiednia platforme w odpowiednim wektorze
	// zwalnia VAO, VBO i takie tam i tworzy na nowo
}

void Level::restartLevel()
{
	restartCount++;
	distributeModelPositions();

	// czyscimy tablice objectsCollected
	collectedCount = 0;
	for (unsigned int i = 0; i < objectsCollected.size(); i++)
		objectsCollected[i] = false;
}

void Level::saveLevel()
{
	// zapisuje do pliku o jakiejs domyslnej nazwie dane poziomu

	//std::ifstream in("resources/levels/" + name + ".levelgeo");
	//std::string line = "";

	//std::getline(in, line);
	//while (line != "")
	//{
	//	std::istringstream splitter(line);
	//	std::string lineType;
	//	splitter >> lineType;

	//	if (lineType == "pltfrm")
	//	{
	//		platform readPlatform;
	//		unsigned int readType;

	//		splitter >> readPlatform.pos.x >> readPlatform.pos.y >> readPlatform.pos.z
	//			>> readPlatform.dim.x >> readPlatform.dim.y >> readPlatform.dim.z
	//			>> readType;

	//		platforms[readType].push_back(readPlatform);
	//	}
	//	else if (lineType == "player")
	//	{
	//		splitter >> initialPlayerPos.x >> initialPlayerPos.y >> initialPlayerPos.z >> initialPlayerYaw;

	//		initialPlayerYaw = glm::radians(initialPlayerYaw);
	//	}
	//	else if (lineType == "object")
	//	{
	//		glm::vec3 readPos;
	//		splitter >> readPos.x >> readPos.y >> readPos.z;

	//		initialObjectPositions.push_back(readPos);
	//	}
	//	else if (lineType == "camera")
	//	{
	//		glm::vec3 readPos;
	//		splitter >> readPos.x >> readPos.y >> readPos.z;

	//		trackingCams.push_back(readPos);
	//	}

	//	std::getline(in, line);
	//}
}

void Level::collidePlayer(Player *p, float deltaTime)
{
	p->vel.y -= cnst::gravity * deltaTime;
	p->pos += p->vel * deltaTime;

	if (!p->onGround)
		p->pos += p->dir * cnst::airControl;

	p->onGround = false;
	p->closeToWall = false;

	glm::vec3 cor;
	for (int m = 0; m < 3; m++)
	{
		for (unsigned int n = 0; n < platforms[m].size(); n++)
		{
			bool closeTest = false;
			if (collideSingle(p->pos, 1.0f, cnst::playerHeight, platforms[m][n], &cor, &closeTest))
			{
				if(cor.y > 0.0f && p->vel.y < 0.0f || cor.y < 0.0f && p->vel.y > 0.0f)
					p->vel.y = 0.0f;

				if (cor.y > 0.0f)
					p->onGround = true;

				if (closeTest && cor.y == 0.0f)
					p->closeToWall = true;

				p->pos += cor;
			}
		}
	}
}

void Level::collideObjects(float deltaTime)
{
	glm::vec3 cor;
	for (unsigned int i = 0; i < models.size(); i++)
	{
		for (unsigned int j = 0; j < models[i]->positions.size(); j++)
		{
			models[i]->velocities[j].y -= cnst::gravity * deltaTime;
			models[i]->positions[j] += models[i]->velocities[j] * deltaTime;

			for (int m = 0; m < 3; m++)
			{
				for (unsigned int n = 0; n < platforms[m].size(); n++)
				{
					bool closeTest = false;
					if (collideSingle(models[i]->positions[j], cnst::objectRadius / 2,
						cnst::objectRadius + cnst::objectFloatHeight, platforms[m][n], &cor, &closeTest))
					{
						// zaczepiaja sie o narozniki
						if (cor.y == 0.0f && cor.x != 0.0f && cor.z != 0.0f)
							cor *= 1.001f;

						models[i]->velocities[j] = glm::vec3(0.0f);
						models[i]->positions[j] += cor;
					}
				}
			}
		}
	}
}

glm::vec3 Level::collideHeld(glm::vec3 pos)
{
	glm::vec3 cor;
	for (int m = 0; m < 3; m++)
	{
		for (unsigned int n = 0; n < platforms[m].size(); n++)
		{
			bool closeTest = false;
			if (collideSingle(pos, cnst::objectRadius / 2,
				cnst::objectRadius + cnst::objectFloatHeight, platforms[m][n], &cor, &closeTest))
			{
				pos += cor;
			}
		}
	}

	return pos;
}

void Level::collectObject(Player *p)
{
	for (unsigned int i = 0; i < models.size(); i++)
	{
		for (unsigned int j = 0; j < models[i]->positions.size(); j++)
		{
			if (!p->holdingObject || (p->heldObjectModel != models[i]) || (p->heldObjectIndex != j))
			{
				// mozemy zebrac ten obiekt jesli jest wystarczajaco blisko sfery
				glm::vec3 objectGrabPos(models[i]->positions[j]);
				objectGrabPos.y += cnst::objectFloatHeight + 0.5f * cnst::objectRadius;
				if (glm::length(objectGrabPos) < cnst::sphereRadius + cnst::objectRadius * 0.5f)
				{
					unsigned int collectedIndex = models[i]->removePosition(j);
					objectsCollected[collectedIndex] = true;
					collectedCount++;
				}
			}
		}
	}
}

bool Level::collideSingle(glm::vec3 pos, float r, float h, platform plt, glm::vec3 *cor, bool *closeToWall)
{
	*cor = glm::vec3(0.0f);
	bool foundCol = false;
	if (!(pos.y < plt.pos.y + plt.dim.y && pos.y + h > plt.pos.y))
		return foundCol;

	bool inXZBands = false;

	if (pos.x >= plt.pos.x && pos.x <= plt.pos.x + plt.dim.x)
	{
		inXZBands = true;

		// czy jestesmy blisko sciany?
		if (pos.z + r + cnst::wallDistance > plt.pos.z && pos.z - r - cnst::wallDistance < plt.pos.z + plt.dim.z)
			*closeToWall = true;

		if (pos.z + r > plt.pos.z && pos.z - r < plt.pos.z + plt.dim.z)
		{
			foundCol = true;
			cor->z = plt.pos.z - (pos.z + r);
			glm::vec3 other(0.0f, 0.0f, plt.pos.z + plt.dim.z - (pos.z - r));
			if (glm::length(other) < glm::length(*cor))
				*cor = other;
		}
	}

	if (pos.z >= plt.pos.z && pos.z <= plt.pos.z + plt.dim.z)
	{
		inXZBands = true;

		// czy jestesmy blisko sciany?
		if (pos.x + r + cnst::wallDistance > plt.pos.x && pos.x - r - cnst::wallDistance < plt.pos.x + plt.dim.x)
			*closeToWall = true;

		if (pos.x + r > plt.pos.x && pos.x - r < plt.pos.x + plt.dim.x)
		{
			glm::vec3 first(plt.pos.x - (pos.x + r), 0.0f, 0.0f);
			if (!foundCol || (foundCol && glm::length(first) < glm::length(*cor)))
			{
				foundCol = true;
				*cor = first;
			}
			glm::vec3 other(plt.pos.x + plt.dim.x - (pos.x - r), 0.0f, 0.0f);
			if (!foundCol || (foundCol && glm::length(other) < glm::length(*cor)))
			{
				foundCol = true;
				*cor = other;
			}
		}
	}

	if (!inXZBands)
	{
		glm::vec3 corners[4] = { glm::vec3(plt.pos.x, 0.0f, plt.pos.z),
								 glm::vec3(plt.pos.x + plt.dim.x, 0.0f, plt.pos.z),
								 glm::vec3(plt.pos.x, 0.0f, plt.pos.z + plt.dim.z),
								 glm::vec3(plt.pos.x + plt.dim.x, 0.0f, plt.pos.z + plt.dim.z), };

		// dla kazdego z 4 wierzcholkow
		for (int i = 0; i < 4; i++)
		{
			glm::vec3 diff(pos.x - corners[i].x, 0.0f, pos.z - corners[i].z);
			float diffLength = glm::length(diff);

			// czy jestesmy blisko sciany?
			if (diffLength < r + cnst::wallDistance)
				*closeToWall = true;

			if (diffLength < r)
			{
				glm::vec3 first = glm::normalize(diff) * (r - diffLength);
				if (!foundCol || (foundCol && glm::length(first) < glm::length(*cor)))
				{
					foundCol = true;
					*cor = first;
				}
			}
		}
	}

	// znalezc tez poprawke pionowa jesli jest kolizja
	if (foundCol)
	{
		glm::vec3 first(0.0f, plt.pos.y - (pos.y + h), 0.0f);
		if (foundCol && glm::length(first) < glm::length(*cor))
			*cor = first;
		glm::vec3 other(0.0f, plt.pos.y + plt.dim.y - pos.y, 0.0f);
		if (foundCol && glm::length(other) < glm::length(*cor))
			*cor = other;
	}

	return foundCol;
}

void Level::genPlatforms(unsigned int type)
{
	// tworzymy tablice wierzcholkow, pozycji tekstur i normali
	// tekstury maja sie powtarzac wiec ustawiamy je na pozycje wierzcholkow XZ, XY albo YZ wymnozone przez skale w cnst
	// poza tym to po prostu rozciagniete szesciany

	// 8 floatow na wierzcholek, 36 wierzcholkow na platforme
	platformVertSize[type] = sizeof(float) * platforms[type].size() * 36 * 8;
	platformVert[type] = new float[platforms[type].size() * 36 * 8];

	for (unsigned int i = 0; i < platforms[type].size(); i++)
	{
		// kierunek (-1,  0,  0)
		// trojkat 1
		platformVert[type][i * 36 * 8 +  0 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  0 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 +  0 * 8 + 2] = platforms[type][i].pos.z;
							   
		platformVert[type][i * 36 * 8 +  0 * 8 + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  0 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  
		platformVert[type][i * 36 * 8 +  0 * 8 + 5] = -1.0f;
		platformVert[type][i * 36 * 8 +  0 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  0 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 +  1 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  1 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 +  1 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
						      			 
		platformVert[type][i * 36 * 8 +  1 * 8 + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  1 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
						     			 
		platformVert[type][i * 36 * 8 +  1 * 8 + 5] = -1.0f;
		platformVert[type][i * 36 * 8 +  1 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  1 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 +  2 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  2 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 +  2 * 8 + 2] = platforms[type][i].pos.z;
						      			 
		platformVert[type][i * 36 * 8 +  2 * 8 + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  2 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
						     			 
		platformVert[type][i * 36 * 8 +  2 * 8 + 5] = -1.0f;
		platformVert[type][i * 36 * 8 +  2 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  2 * 8 + 7] =  0.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 +  3 * 8 + 0] = platformVert[type][i * 36 * 8 +  0 * 8 + 0];
		platformVert[type][i * 36 * 8 +  3 * 8 + 1] = platformVert[type][i * 36 * 8 +  0 * 8 + 1];
		platformVert[type][i * 36 * 8 +  3 * 8 + 2] = platformVert[type][i * 36 * 8 +  0 * 8 + 2];
							   		     
		platformVert[type][i * 36 * 8 +  3 * 8 + 3] = platformVert[type][i * 36 * 8 +  0 * 8 + 3];
		platformVert[type][i * 36 * 8 +  3 * 8 + 4] = platformVert[type][i * 36 * 8 +  0 * 8 + 4];
							  			    
		platformVert[type][i * 36 * 8 +  3 * 8 + 5] = platformVert[type][i * 36 * 8 +  0 * 8 + 5];
		platformVert[type][i * 36 * 8 +  3 * 8 + 6] = platformVert[type][i * 36 * 8 +  0 * 8 + 6];
		platformVert[type][i * 36 * 8 +  3 * 8 + 7] = platformVert[type][i * 36 * 8 +  0 * 8 + 7];

		platformVert[type][i * 36 * 8 +  4 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  4 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 +  4 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			 
		platformVert[type][i * 36 * 8 +  4 * 8 + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  4 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 +  4 * 8 + 5] = -1.0f;
		platformVert[type][i * 36 * 8 +  4 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  4 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 +  5 * 8 + 0] = platformVert[type][i * 36 * 8 +  1 * 8 + 0];
		platformVert[type][i * 36 * 8 +  5 * 8 + 1] = platformVert[type][i * 36 * 8 +  1 * 8 + 1];
		platformVert[type][i * 36 * 8 +  5 * 8 + 2] = platformVert[type][i * 36 * 8 +  1 * 8 + 2];
							   											    		   
		platformVert[type][i * 36 * 8 +  5 * 8 + 3] = platformVert[type][i * 36 * 8 +  1 * 8 + 3];
		platformVert[type][i * 36 * 8 +  5 * 8 + 4] = platformVert[type][i * 36 * 8 +  1 * 8 + 4];
							  											   			   
		platformVert[type][i * 36 * 8 +  5 * 8 + 5] = platformVert[type][i * 36 * 8 +  1 * 8 + 5];
		platformVert[type][i * 36 * 8 +  5 * 8 + 6] = platformVert[type][i * 36 * 8 +  1 * 8 + 6];
		platformVert[type][i * 36 * 8 +  5 * 8 + 7] = platformVert[type][i * 36 * 8 +  1 * 8 + 7];
		
		// kierunek ( 0,  0,  1)
		// trojkat 1
		platformVert[type][i * 36 * 8 +  6 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  6 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 +  6 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			 
		platformVert[type][i * 36 * 8 +  6 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  6 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 +  6 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 +  6 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  6 * 8 + 7] =  1.0f;

		platformVert[type][i * 36 * 8 +  7 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 +  7 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 +  7 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			 
		platformVert[type][i * 36 * 8 +  7 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  7 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 +  7 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 +  7 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  7 * 8 + 7] =  1.0f;

		platformVert[type][i * 36 * 8 +  8 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 +  8 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 +  8 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			 
		platformVert[type][i * 36 * 8 +  8 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 +  8 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 +  8 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 +  8 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 +  8 * 8 + 7] =  1.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 +  9 * 8 + 0] = platformVert[type][i * 36 * 8 +  6 * 8 + 0];
		platformVert[type][i * 36 * 8 +  9 * 8 + 1] = platformVert[type][i * 36 * 8 +  6 * 8 + 1];
		platformVert[type][i * 36 * 8 +  9 * 8 + 2] = platformVert[type][i * 36 * 8 +  6 * 8 + 2];
							   				  							    		  
		platformVert[type][i * 36 * 8 +  9 * 8 + 3] = platformVert[type][i * 36 * 8 +  6 * 8 + 3];
		platformVert[type][i * 36 * 8 +  9 * 8 + 4] = platformVert[type][i * 36 * 8 +  6 * 8 + 4];
							  					  						   			  
		platformVert[type][i * 36 * 8 +  9 * 8 + 5] = platformVert[type][i * 36 * 8 +  6 * 8 + 5];
		platformVert[type][i * 36 * 8 +  9 * 8 + 6] = platformVert[type][i * 36 * 8 +  6 * 8 + 6];
		platformVert[type][i * 36 * 8 +  9 * 8 + 7] = platformVert[type][i * 36 * 8 +  6 * 8 + 7];

		platformVert[type][i * 36 * 8 + 10 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 10 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 10 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			
		platformVert[type][i * 36 * 8 + 10 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 10 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			
		platformVert[type][i * 36 * 8 + 10 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 10 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 10 * 8 + 7] =  1.0f;

		platformVert[type][i * 36 * 8 + 11 * 8 + 0] = platformVert[type][i * 36 * 8 +  7 * 8 + 0];
		platformVert[type][i * 36 * 8 + 11 * 8 + 1] = platformVert[type][i * 36 * 8 +  7 * 8 + 1];
		platformVert[type][i * 36 * 8 + 11 * 8 + 2] = platformVert[type][i * 36 * 8 +  7 * 8 + 2];
																					  
		platformVert[type][i * 36 * 8 + 11 * 8 + 3] = platformVert[type][i * 36 * 8 +  7 * 8 + 3];
		platformVert[type][i * 36 * 8 + 11 * 8 + 4] = platformVert[type][i * 36 * 8 +  7 * 8 + 4];
																					  
		platformVert[type][i * 36 * 8 + 11 * 8 + 5] = platformVert[type][i * 36 * 8 +  7 * 8 + 5];
		platformVert[type][i * 36 * 8 + 11 * 8 + 6] = platformVert[type][i * 36 * 8 +  7 * 8 + 6];
		platformVert[type][i * 36 * 8 + 11 * 8 + 7] = platformVert[type][i * 36 * 8 +  7 * 8 + 7];
		
		// kierunek ( 1,  0,  0)
		// trojkat 1
		platformVert[type][i * 36 * 8 + 12 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 12 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 12 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
										 
		platformVert[type][i * 36 * 8 + 12 * 8 + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 12 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
										 
		platformVert[type][i * 36 * 8 + 12 * 8 + 5] =  1.0f;
		platformVert[type][i * 36 * 8 + 12 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 12 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 13 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 13 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 13 * 8 + 2] = platforms[type][i].pos.z;
							   			 
		platformVert[type][i * 36 * 8 + 13 * 8 + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 13 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 + 13 * 8 + 5] =  1.0f;
		platformVert[type][i * 36 * 8 + 13 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 13 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 14 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 14 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 14 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   			 
		platformVert[type][i * 36 * 8 + 14 * 8 + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 14 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 + 14 * 8 + 5] =  1.0f;
		platformVert[type][i * 36 * 8 + 14 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 14 * 8 + 7] =  0.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 + 15 * 8 + 0] = platformVert[type][i * 36 * 8 + 12 * 8 + 0];
		platformVert[type][i * 36 * 8 + 15 * 8 + 1] = platformVert[type][i * 36 * 8 + 12 * 8 + 1];
		platformVert[type][i * 36 * 8 + 15 * 8 + 2] = platformVert[type][i * 36 * 8 + 12 * 8 + 2];
							   									 		    		
		platformVert[type][i * 36 * 8 + 15 * 8 + 3] = platformVert[type][i * 36 * 8 + 12 * 8 + 3];
		platformVert[type][i * 36 * 8 + 15 * 8 + 4] = platformVert[type][i * 36 * 8 + 12 * 8 + 4];
							  								 			   			 
		platformVert[type][i * 36 * 8 + 15 * 8 + 5] = platformVert[type][i * 36 * 8 + 12 * 8 + 5];
		platformVert[type][i * 36 * 8 + 15 * 8 + 6] = platformVert[type][i * 36 * 8 + 12 * 8 + 6];
		platformVert[type][i * 36 * 8 + 15 * 8 + 7] = platformVert[type][i * 36 * 8 + 12 * 8 + 7];

		platformVert[type][i * 36 * 8 + 16 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 16 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 16 * 8 + 2] = platforms[type][i].pos.z;
							   			 
		platformVert[type][i * 36 * 8 + 16 * 8 + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 16 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 + 16 * 8 + 5] =  1.0f;
		platformVert[type][i * 36 * 8 + 16 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 16 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 17 * 8 + 0] = platformVert[type][i * 36 * 8 + 13 * 8 + 0];
		platformVert[type][i * 36 * 8 + 17 * 8 + 1] = platformVert[type][i * 36 * 8 + 13 * 8 + 1];
		platformVert[type][i * 36 * 8 + 17 * 8 + 2] = platformVert[type][i * 36 * 8 + 13 * 8 + 2];
							   					  		 				    	
		platformVert[type][i * 36 * 8 + 17 * 8 + 3] = platformVert[type][i * 36 * 8 + 13 * 8 + 3];
		platformVert[type][i * 36 * 8 + 17 * 8 + 4] = platformVert[type][i * 36 * 8 + 13 * 8 + 4];
							  					  			 			   			
		platformVert[type][i * 36 * 8 + 17 * 8 + 5] = platformVert[type][i * 36 * 8 + 13 * 8 + 5];
		platformVert[type][i * 36 * 8 + 17 * 8 + 6] = platformVert[type][i * 36 * 8 + 13 * 8 + 6];
		platformVert[type][i * 36 * 8 + 17 * 8 + 7] = platformVert[type][i * 36 * 8 + 13 * 8 + 7];
		
		// kierunek ( 0,  0, -1)
		// trojkat 1
		platformVert[type][i * 36 * 8 + 18 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 18 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 18 * 8 + 2] = platforms[type][i].pos.z;
							   			 
		platformVert[type][i * 36 * 8 + 18 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 18 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 + 18 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 18 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 18 * 8 + 7] = -1.0f;

		platformVert[type][i * 36 * 8 + 19 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 19 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 19 * 8 + 2] = platforms[type][i].pos.z;
							   			 
		platformVert[type][i * 36 * 8 + 19 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 19 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			 
		platformVert[type][i * 36 * 8 + 19 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 19 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 19 * 8 + 7] = -1.0f;

		platformVert[type][i * 36 * 8 + 20 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 20 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 20 * 8 + 2] = platforms[type][i].pos.z;
							   			
		platformVert[type][i * 36 * 8 + 20 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 20 * 8 + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;
							  			
		platformVert[type][i * 36 * 8 + 20 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 20 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 20 * 8 + 7] = -1.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 + 21 * 8 + 0] = platformVert[type][i * 36 * 8 + 18 * 8 + 0];
		platformVert[type][i * 36 * 8 + 21 * 8 + 1] = platformVert[type][i * 36 * 8 + 18 * 8 + 1];
		platformVert[type][i * 36 * 8 + 21 * 8 + 2] = platformVert[type][i * 36 * 8 + 18 * 8 + 2];
							   						 	 				    		
		platformVert[type][i * 36 * 8 + 21 * 8 + 3] = platformVert[type][i * 36 * 8 + 18 * 8 + 3];
		platformVert[type][i * 36 * 8 + 21 * 8 + 4] = platformVert[type][i * 36 * 8 + 18 * 8 + 4];
							  						 	 				   			  
		platformVert[type][i * 36 * 8 + 21 * 8 + 5] = platformVert[type][i * 36 * 8 + 18 * 8 + 5];
		platformVert[type][i * 36 * 8 + 21 * 8 + 6] = platformVert[type][i * 36 * 8 + 18 * 8 + 6];
		platformVert[type][i * 36 * 8 + 21 * 8 + 7] = platformVert[type][i * 36 * 8 + 18 * 8 + 7];

		platformVert[type][i * 36 * 8 + 22 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 22 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 22 * 8 + 2] = platforms[type][i].pos.z;
							   		
		platformVert[type][i * 36 * 8 + 22 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 22 * 8 + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;
							  			
		platformVert[type][i * 36 * 8 + 22 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 22 * 8 + 6] =  0.0f;
		platformVert[type][i * 36 * 8 + 22 * 8 + 7] = -1.0f;

		platformVert[type][i * 36 * 8 + 23 * 8 + 0] = platformVert[type][i * 36 * 8 + 19 * 8 + 0];
		platformVert[type][i * 36 * 8 + 23 * 8 + 1] = platformVert[type][i * 36 * 8 + 19 * 8 + 1];
		platformVert[type][i * 36 * 8 + 23 * 8 + 2] = platformVert[type][i * 36 * 8 + 19 * 8 + 2];
							   		 		 			  				    		
		platformVert[type][i * 36 * 8 + 23 * 8 + 3] = platformVert[type][i * 36 * 8 + 19 * 8 + 3];
		platformVert[type][i * 36 * 8 + 23 * 8 + 4] = platformVert[type][i * 36 * 8 + 19 * 8 + 4];
							    			 			  				   			  
		platformVert[type][i * 36 * 8 + 23 * 8 + 5] = platformVert[type][i * 36 * 8 + 19 * 8 + 5];
		platformVert[type][i * 36 * 8 + 23 * 8 + 6] = platformVert[type][i * 36 * 8 + 19 * 8 + 6];
		platformVert[type][i * 36 * 8 + 23 * 8 + 7] = platformVert[type][i * 36 * 8 + 19 * 8 + 7];
		
		// kierunek ( 0, -1,  0)
		// trojkat 1
		platformVert[type][i * 36 * 8 + 24 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 24 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 24 * 8 + 2] = platforms[type][i].pos.z;
							   		
		platformVert[type][i * 36 * 8 + 24 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 24 * 8 + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;
							  		
		platformVert[type][i * 36 * 8 + 24 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 24 * 8 + 6] = -1.0f;
		platformVert[type][i * 36 * 8 + 24 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 25 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 25 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 25 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   	
		platformVert[type][i * 36 * 8 + 25 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 25 * 8 + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
							  	
		platformVert[type][i * 36 * 8 + 25 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 25 * 8 + 6] = -1.0f;
		platformVert[type][i * 36 * 8 + 25 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 26 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 26 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 26 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   	
		platformVert[type][i * 36 * 8 + 26 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 26 * 8 + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
							  	
		platformVert[type][i * 36 * 8 + 26 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 26 * 8 + 6] = -1.0f;
		platformVert[type][i * 36 * 8 + 26 * 8 + 7] =  0.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 + 27 * 8 + 0] = platformVert[type][i * 36 * 8 + 24 * 8 + 0];
		platformVert[type][i * 36 * 8 + 27 * 8 + 1] = platformVert[type][i * 36 * 8 + 24 * 8 + 1];
		platformVert[type][i * 36 * 8 + 27 * 8 + 2] = platformVert[type][i * 36 * 8 + 24 * 8 + 2];
							   			 					 			    
		platformVert[type][i * 36 * 8 + 27 * 8 + 3] = platformVert[type][i * 36 * 8 + 24 * 8 + 3];
		platformVert[type][i * 36 * 8 + 27 * 8 + 4] = platformVert[type][i * 36 * 8 + 24 * 8 + 4];
							  		 					 				   	
		platformVert[type][i * 36 * 8 + 27 * 8 + 5] = platformVert[type][i * 36 * 8 + 24 * 8 + 5];
		platformVert[type][i * 36 * 8 + 27 * 8 + 6] = platformVert[type][i * 36 * 8 + 24 * 8 + 6];
		platformVert[type][i * 36 * 8 + 27 * 8 + 7] = platformVert[type][i * 36 * 8 + 24 * 8 + 7];

		platformVert[type][i * 36 * 8 + 28 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 28 * 8 + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * 8 + 28 * 8 + 2] = platforms[type][i].pos.z;
							   	
		platformVert[type][i * 36 * 8 + 28 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 28 * 8 + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;
							  	
		platformVert[type][i * 36 * 8 + 28 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 28 * 8 + 6] = -1.0f;
		platformVert[type][i * 36 * 8 + 28 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 29 * 8 + 0] = platformVert[type][i * 36 * 8 + 25 * 8 + 0];
		platformVert[type][i * 36 * 8 + 29 * 8 + 1] = platformVert[type][i * 36 * 8 + 25 * 8 + 1];
		platformVert[type][i * 36 * 8 + 29 * 8 + 2] = platformVert[type][i * 36 * 8 + 25 * 8 + 2];
							   			 			  			 		    	
		platformVert[type][i * 36 * 8 + 29 * 8 + 3] = platformVert[type][i * 36 * 8 + 25 * 8 + 3];
		platformVert[type][i * 36 * 8 + 29 * 8 + 4] = platformVert[type][i * 36 * 8 + 25 * 8 + 4];
							 			  			 					   
		platformVert[type][i * 36 * 8 + 29 * 8 + 5] = platformVert[type][i * 36 * 8 + 25 * 8 + 5];
		platformVert[type][i * 36 * 8 + 29 * 8 + 6] = platformVert[type][i * 36 * 8 + 25 * 8 + 6];
		platformVert[type][i * 36 * 8 + 29 * 8 + 7] = platformVert[type][i * 36 * 8 + 25 * 8 + 7];
		
		// kierunek ( 0,  1,  0)
		// trojkat 1
		platformVert[type][i * 36 * 8 + 30 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 30 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 30 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   		
		platformVert[type][i * 36 * 8 + 30 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 30 * 8 + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;
							  		
		platformVert[type][i * 36 * 8 + 30 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 30 * 8 + 6] =  1.0f;
		platformVert[type][i * 36 * 8 + 30 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 31 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 31 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 31 * 8 + 2] = platforms[type][i].pos.z;
							   		
		platformVert[type][i * 36 * 8 + 31 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 31 * 8 + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
							  	
		platformVert[type][i * 36 * 8 + 31 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 31 * 8 + 6] =  1.0f;
		platformVert[type][i * 36 * 8 + 31 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 32 * 8 + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * 8 + 32 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 32 * 8 + 2] = platforms[type][i].pos.z;
							   		
		platformVert[type][i * 36 * 8 + 32 * 8 + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 32 * 8 + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
							  		
		platformVert[type][i * 36 * 8 + 32 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 32 * 8 + 6] =  1.0f;
		platformVert[type][i * 36 * 8 + 32 * 8 + 7] =  0.0f;
		
		// trojkat 2
		platformVert[type][i * 36 * 8 + 33 * 8 + 0] = platformVert[type][i * 36 * 8 + 30 * 8 + 0];
		platformVert[type][i * 36 * 8 + 33 * 8 + 1] = platformVert[type][i * 36 * 8 + 30 * 8 + 1];
		platformVert[type][i * 36 * 8 + 33 * 8 + 2] = platformVert[type][i * 36 * 8 + 30 * 8 + 2];
							   				 			    						
		platformVert[type][i * 36 * 8 + 33 * 8 + 3] = platformVert[type][i * 36 * 8 + 30 * 8 + 3];
		platformVert[type][i * 36 * 8 + 33 * 8 + 4] = platformVert[type][i * 36 * 8 + 30 * 8 + 4];
							  			 				   							
		platformVert[type][i * 36 * 8 + 33 * 8 + 5] = platformVert[type][i * 36 * 8 + 30 * 8 + 5];
		platformVert[type][i * 36 * 8 + 33 * 8 + 6] = platformVert[type][i * 36 * 8 + 30 * 8 + 6];
		platformVert[type][i * 36 * 8 + 33 * 8 + 7] = platformVert[type][i * 36 * 8 + 30 * 8 + 7];

		platformVert[type][i * 36 * 8 + 34 * 8 + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * 8 + 34 * 8 + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * 8 + 34 * 8 + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;
							   	
		platformVert[type][i * 36 * 8 + 34 * 8 + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * 8 + 34 * 8 + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;
							  	
		platformVert[type][i * 36 * 8 + 34 * 8 + 5] =  0.0f;
		platformVert[type][i * 36 * 8 + 34 * 8 + 6] =  1.0f;
		platformVert[type][i * 36 * 8 + 34 * 8 + 7] =  0.0f;

		platformVert[type][i * 36 * 8 + 35 * 8 + 0] = platformVert[type][i * 36 * 8 + 31 * 8 + 0];
		platformVert[type][i * 36 * 8 + 35 * 8 + 1] = platformVert[type][i * 36 * 8 + 31 * 8 + 1];
		platformVert[type][i * 36 * 8 + 35 * 8 + 2] = platformVert[type][i * 36 * 8 + 31 * 8 + 2];
							   			  			 		    		
		platformVert[type][i * 36 * 8 + 35 * 8 + 3] = platformVert[type][i * 36 * 8 + 31 * 8 + 3];
		platformVert[type][i * 36 * 8 + 35 * 8 + 4] = platformVert[type][i * 36 * 8 + 31 * 8 + 4];
							 		 					   			
		platformVert[type][i * 36 * 8 + 35 * 8 + 5] = platformVert[type][i * 36 * 8 + 31 * 8 + 5];
		platformVert[type][i * 36 * 8 + 35 * 8 + 6] = platformVert[type][i * 36 * 8 + 31 * 8 + 6];
		platformVert[type][i * 36 * 8 + 35 * 8 + 7] = platformVert[type][i * 36 * 8 + 31 * 8 + 7];
	}

	// robienie EBO do tego chyba bedzie zbyt skomplikowane, ale nie trzeba
	// bo zrobilismy mozliwosc bindowania VAO VN albo VTN bez indices
	util::bindVAO_VTN(&platformVAO[type], &platformVBO[type], nullptr, platformVert[type], platformVertSize[type], nullptr, 0);
}

void Level::distributeModelPositions()
{
	// czyscimy tablice pozycji w modelach
	for (unsigned int i = 0; i < models.size(); i++)
		models[i]->clearPositions();

	// shufflujemy initialObjectsPositions
	// albo narazie bez bawienia sie w shufflowanie, po prostu reszta z dzielenia
	for (unsigned int i = 0; i < initialObjectPositions.size(); i++)
		models[i % models.size()]->addPosition(initialObjectPositions[i], i);
}
