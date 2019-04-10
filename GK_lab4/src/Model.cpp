#include "Model.h"

#include <cstdlib>
#include <algorithm>

#include "Level.h"
#include "constants.h"
#include "utility.h"
#include "Player.h"

Model::Model(std::string name, unsigned int progVN, unsigned int progVTN, unsigned int progCol)
{
	textured = util::readObj(("resources/models/" + name + ".obj").c_str(), &vert, &vertSize, &indc, &indcSize);
	util::readLights(name, &modelPnts, &modelDirs, false, nullptr, nullptr);
	for (unsigned int i = 0; i < modelDirs.size(); i++)
		modelDirs[i].dir = glm::normalize(modelDirs[i].dir);

	if (!textured)
	{
		untexturedColor.r = (float)(rand() % 256) / 256.0f;
		untexturedColor.g = (float)(rand() % 256) / 256.0f;
		untexturedColor.b = (float)(rand() % 256) / 256.0f;
	}

	progShaderID = progVN;
	if (textured)
		progShaderID = progVTN;
	progColShader = progCol;

	// po wczytaniu znajdujemy najwiekszy wymiar i scale = jego odwrotnosc czy cos w tym stylu
	// (znajdujemy tez najmniejsze Y zeby wszystkie obiekty rowno staly na ziemi)
	findScaleAndBottom();
	rotation = 0.0f;

	if (textured)
	{
		textureID = util::createBasicTexture(("resources/textures/" + name + ".png").c_str());
		util::bindVAO_VTN(&VAO, &VBO, &EBO, vert, vertSize, indc, indcSize);
	}
	else
		util::bindVAO_VN(&VAO, &VBO, &EBO, vert, vertSize, indc, indcSize);

	util::readObj(cnst::cylinderPath, &colVert, &colVertSize, &colIndc, &colIndcSize);
	util::bindVAO_VN(&colVAO, &colVBO, &colEBO, colVert, colVertSize, colIndc, colIndcSize);
}


Model::~Model()
{
	if(textured)
		glDeleteTextures(1, &textureID);

	delete[] vert;
	delete[] indc;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	delete[] colVert;
	delete[] colIndc;
	glDeleteVertexArrays(1, &colVAO);
	glDeleteBuffers(1, &colVBO);
	glDeleteBuffers(1, &colEBO);
}

void Model::draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS, bool wF, bool dC, Level *l)
{
	if (dC)
	{
		if (!wF)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for (unsigned int i = 0; i < positions.size(); i++)
		{
			glUseProgram(progColShader);

			glUniform3f(glGetUniformLocation(progColShader, "objectColor"),
				cnst::objectColColor.r, cnst::objectColColor.g, cnst::objectColColor.b);

			glm::mat4 mdel(1.0f);
			mdel = glm::translate(mdel, positions[i]);
			//mdel = glm::rotate(mdel, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
			mdel = glm::scale(mdel, glm::vec3(cnst::objectRadius / 2,
				(cnst::objectFloatHeight + cnst::objectRadius) / 2, cnst::objectRadius / 2));
			mdel = glm::translate(mdel, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::mat4 MVP = proj * view * mdel;
			glUniformMatrix4fv(glGetUniformLocation(progColShader, "mvp"), 1, GL_FALSE, glm::value_ptr(MVP));

			glBindVertexArray(colVAO);
			glDrawElements(GL_TRIANGLES, colIndcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		}

		if (!wF)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(progShaderID);

	glUniform3f(glGetUniformLocation(progShaderID, "sun.color"), l->sun.color.r, l->sun.color.g, l->sun.color.b);
	glUniform3f(glGetUniformLocation(progShaderID, "sun.dir"), l->sun.dir.x, l->sun.dir.y, l->sun.dir.z);

	glUniform3f(glGetUniformLocation(progShaderID, "ambient"), l->ambient.r, l->ambient.g, l->ambient.b);
	if (textured)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glUniform1i(glGetUniformLocation(progShaderID, "ourTexture"), 0);
	}
	else
		glUniform3f(glGetUniformLocation(progShaderID, "objectColor"), untexturedColor.r, untexturedColor.g, untexturedColor.b);

	glUniform3f(glGetUniformLocation(progShaderID, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

	glUniform1i(glGetUniformLocation(progShaderID, "pntSize"), l->globalPntsCount);
	glUniform1i(glGetUniformLocation(progShaderID, "dirSize"), l->globalDirsCount);
	for (unsigned int i = 0; i < l->globalPntsCount; i++)
	{
		std::string pntName = "pnts[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progShaderID, (pntName + ".pos").c_str()),
			l->globalPnts[i].pos.x, l->globalPnts[i].pos.y, l->globalPnts[i].pos.z);
		glUniform3f(glGetUniformLocation(progShaderID, (pntName + ".color").c_str()),
			l->globalPnts[i].color.r, l->globalPnts[i].color.g, l->globalPnts[i].color.b);
	}
	for (unsigned int i = 0; i < l->globalDirsCount; i++)
	{
		std::string dirName = "dirs[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progShaderID, (dirName + ".pos").c_str()),
			l->globalDirs[i].pos.x, l->globalDirs[i].pos.y, l->globalDirs[i].pos.z);
		glUniform3f(glGetUniformLocation(progShaderID, (dirName + ".dir").c_str()),
			l->globalDirs[i].dir.x, l->globalDirs[i].dir.y, l->globalDirs[i].dir.z);
		glUniform1i(glGetUniformLocation(progShaderID, (dirName + ".focus").c_str()), l->globalDirs[i].focus);
		glUniform3f(glGetUniformLocation(progShaderID, (dirName + ".color").c_str()),
			l->globalDirs[i].color.r, l->globalDirs[i].color.g, l->globalDirs[i].color.b);
	}

	glUniformMatrix4fv(glGetUniformLocation(progShaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(progShaderID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

	glUniform1i(glGetUniformLocation(progShaderID, "blinnLighting"), bL);
	glUniform1i(glGetUniformLocation(progShaderID, "gouraudShading"), gS);

	glBindVertexArray(VAO);

	for (unsigned int p = 0; p < positions.size(); p++)
	{
		glm::mat4 mdel(1.0f);
		mdel = glm::translate(mdel, glm::vec3(positions[p].x, positions[p].y + cnst::objectFloatHeight, positions[p].z));
		mdel = glm::rotate(mdel, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = 20.0f * p;
		mdel = glm::rotate(mdel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f)); // os nie musi byc jednostkowa
		mdel = glm::scale(mdel, glm::vec3(scale, scale, scale));
		mdel = glm::translate(mdel, glm::vec3(0.0f, 0.0f - bottom, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(progShaderID, "mdel"), 1, GL_FALSE, glm::value_ptr(mdel));

		glm::mat3 norm(glm::transpose(glm::inverse(mdel)));
		glUniformMatrix3fv(glGetUniformLocation(progShaderID, "norm"), 1, GL_FALSE, glm::value_ptr(norm));

		glDrawElements(GL_TRIANGLES, indcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
	}
}

void Model::updateRotation(float deltaTime)
{
	rotation += cnst::objectRotationSpeed * deltaTime;
}

bool Model::addLightsToGlobal(Level *l)
{
	std::vector<pntLight> allPosPnts;
	std::vector<dirLight> allPosDirs;

	for (unsigned int p = 0; p < positions.size(); p++)
	{
		glm::mat4 mdel = glm::mat4(1.0f);
		mdel = glm::translate(mdel, glm::vec3(positions[p].x, positions[p].y + cnst::objectFloatHeight, positions[p].z));
		mdel = glm::rotate(mdel, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = 20.0f * p;
		mdel = glm::rotate(mdel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f)); // os nie musi byc jednostkowa
		mdel = glm::scale(mdel, glm::vec3(scale, scale, scale));
		mdel = glm::translate(mdel, glm::vec3(0.0f, 0.0f - bottom, 0.0f));
		glm::mat3 norm(glm::transpose(glm::inverse(mdel)));

		for (unsigned int l = 0; l < modelPnts.size(); l++)
		{
			pntLight transformed = modelPnts[l];
			transformed.pos = mdel * glm::vec4(transformed.pos, 1.0f);

			allPosPnts.push_back(transformed);
		}

		for (unsigned int l = 0; l < modelDirs.size(); l++)
		{
			dirLight transformed = modelDirs[l];
			transformed.pos = mdel * glm::vec4(transformed.pos, 1.0f);
			transformed.dir = glm::normalize(norm * transformed.dir);

			allPosDirs.push_back(transformed);
		}
	}

	return l->addLights(allPosPnts, allPosDirs);
}

void Model::addPosition(glm::vec3 pos, unsigned int i)
{
	positions.push_back(pos);
	initialPositions.push_back(pos);
	velocities.push_back(glm::vec3(0.0f));
	posIndex.push_back(i);
}

void Model::clearPositions()
{
	positions.clear();
	initialPositions.clear();
	velocities.clear();
	posIndex.clear();
}

unsigned int Model::removePosition(unsigned int i)
{
	positions.erase(positions.begin() + i);
	initialPositions.erase(initialPositions.begin() + i);
	velocities.erase(velocities.begin() + i);

	unsigned int index = posIndex[i];
	posIndex.erase(posIndex.begin() + i);
	return index;
}

void Model::checkPositions()
{
	for (unsigned int i = 0; i < positions.size(); i++)
	{
		if (positions[i].y <= cnst::barrierHeight)
		{
			positions[i] = initialPositions[i];
			velocities[i] = glm::vec3(0.0f);
		}
	}
}

void Model::findScaleAndBottom()
{
	// odwrotnosc najwiekszego rozmiaru
	// znajdujemy najwiekszy i najmniejszy X, Y, Z przeszukujac vert
	unsigned int vertCount = vertSize / sizeof(float);
	unsigned int singleVert = 6;
	if (textured)
		singleVert = 8;
	vertCount /= singleVert;

	float minX = vert[0 * singleVert + 0], maxX = vert[0 * singleVert + 0];
	float minY = vert[0 * singleVert + 1], maxY = vert[0 * singleVert + 1];
	float minZ = vert[0 * singleVert + 2], maxZ = vert[0 * singleVert + 2];
	for (unsigned int v = 1; v < vertCount; v++)
	{
		if (vert[v * singleVert + 0] < minX)
			minX = vert[v * singleVert + 0];
		if (vert[v * singleVert + 0] > maxX)
			maxX = vert[v * singleVert + 0];

		if (vert[v * singleVert + 1] < minY)
			minY = vert[v * singleVert + 1];
		if (vert[v * singleVert + 1] > maxY)
			maxY = vert[v * singleVert + 1];

		if (vert[v * singleVert + 2] < minZ)
			minZ = vert[v * singleVert + 2];
		if (vert[v * singleVert + 2] > maxZ)
			maxZ = vert[v * singleVert + 2];
	}

	float maxDim = std::max(maxX - minX, std::max(maxY - minY, maxZ - minZ));

	scale = cnst::objectRadius / maxDim;
	bottom = minY; // *scale;
}
