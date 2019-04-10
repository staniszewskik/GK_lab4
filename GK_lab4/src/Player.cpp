#include "Player.h"

#include "utility.h"
#include "constants.h"
#include "Model.h"
#include "Level.h"

Player::Player(unsigned int progVN, unsigned int progCol)
{
	util::readObj(cnst::playerPath, &vert, &vertSize, &indc, &indcSize);

	progVNShader = progVN;
	progColShader = progCol;

	heldObjectIndex = 0;
	heldObjectModel = nullptr;
	holdingObject = false;

	onGround = false;
	closeToWall = false;

	util::bindVAO_VN(&VAO, &VBO, &EBO, vert, vertSize, indc, indcSize);

	util::readObj(cnst::cylinderPath, &colVert, &colVertSize, &colIndc, &colIndcSize);
	util::bindVAO_VN(&colVAO, &colVBO, &colEBO, colVert, colVertSize, colIndc, colIndcSize);
}

Player::~Player()
{
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

void Player::draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, bool bL, bool gS, Level *l)
{
	glUseProgram(progVNShader);

	glUniform3f(glGetUniformLocation(progVNShader, "sun.color"), l->sun.color.r, l->sun.color.g, l->sun.color.b);
	glUniform3f(glGetUniformLocation(progVNShader, "sun.dir"), l->sun.dir.x, l->sun.dir.y, l->sun.dir.z);

	glUniform3f(glGetUniformLocation(progVNShader, "ambient"), l->ambient.r, l->ambient.g, l->ambient.b);
	glUniform3f(glGetUniformLocation(progVNShader, "objectColor"), cnst::playerColor.r, cnst::playerColor.g, cnst::playerColor.b);

	glUniform3f(glGetUniformLocation(progVNShader, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

	glUniform1i(glGetUniformLocation(progVNShader, "pntSize"), l->globalPntsCount);
	glUniform1i(glGetUniformLocation(progVNShader, "dirSize"), l->globalDirsCount);
	for (unsigned int i = 0; i < l->globalPntsCount; i++)
	{
		std::string pntName = "pnts[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVNShader, (pntName + ".pos").c_str()),
			l->globalPnts[i].pos.x, l->globalPnts[i].pos.y, l->globalPnts[i].pos.z);
		glUniform3f(glGetUniformLocation(progVNShader, (pntName + ".color").c_str()),
			l->globalPnts[i].color.r, l->globalPnts[i].color.g, l->globalPnts[i].color.b);
	}
	for (unsigned int i = 0; i < l->globalDirsCount; i++)
	{
		std::string dirName = "dirs[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".pos").c_str()),
			l->globalDirs[i].pos.x, l->globalDirs[i].pos.y, l->globalDirs[i].pos.z);
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".dir").c_str()),
			l->globalDirs[i].dir.x, l->globalDirs[i].dir.y, l->globalDirs[i].dir.z);
		glUniform1i(glGetUniformLocation(progVNShader, (dirName + ".focus").c_str()), l->globalDirs[i].focus);
		glUniform3f(glGetUniformLocation(progVNShader, (dirName + ".color").c_str()),
			l->globalDirs[i].color.r, l->globalDirs[i].color.g, l->globalDirs[i].color.b);
	}

	glm::mat4 mdel(1.0f);
	mdel = glm::translate(mdel, pos);
	mdel = glm::rotate(mdel, pyr.y + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "mdel"), 1, GL_FALSE, glm::value_ptr(mdel));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(progVNShader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glm::mat3 norm(glm::transpose(glm::inverse(mdel)));
	glUniformMatrix3fv(glGetUniformLocation(progVNShader, "norm"), 1, GL_FALSE, glm::value_ptr(norm));

	glUniform1i(glGetUniformLocation(progVNShader, "blinnLighting"), bL);
	glUniform1i(glGetUniformLocation(progVNShader, "gouraudShading"), gS);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void Player::drawCol(glm::mat4 view, glm::mat4 proj)
{
	glUseProgram(progColShader);

	glUniform3f(glGetUniformLocation(progColShader, "objectColor"), cnst::playerColor.r, cnst::playerColor.g, cnst::playerColor.b);

	glm::mat4 mdel(1.0f);
	mdel = glm::translate(mdel, pos);
	//mdel = glm::rotate(mdel, pyr.y + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mdel = glm::scale(mdel, glm::vec3(1.0f, cnst::playerHeight / 2, 1.0f));
	mdel = glm::translate(mdel, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 MVP = proj * view * mdel;
	glUniformMatrix4fv(glGetUniformLocation(progColShader, "mvp"), 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(colVAO);
	glDrawElements(GL_TRIANGLES, colIndcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void Player::grabObject(std::vector<Model *> models)
{
	for (unsigned int m = 0; m < models.size(); m++)
	{
		for (unsigned int n = 0; n < models[m]->positions.size(); n++)
		{
			glm::vec3 playerGrabPos(pos);
			playerGrabPos.y += 0.75f * cnst::playerHeight;
			glm::vec3 objectGrabPos(models[m]->positions[n]);
			objectGrabPos.y += cnst::objectFloatHeight + 0.5f * cnst::objectRadius;
			if (glm::distance(playerGrabPos, objectGrabPos) < 1.0f + cnst::objectRadius * 0.5f)
			{
				holdingObject = true;
				heldObjectModel = models[m];
				heldObjectIndex = n;

				goto grabbedObject;
			}
		}
	}

grabbedObject:
	return;
}

void Player::collideSphere()
{
	glm::vec3 playerFirstPos(pos);
	playerFirstPos.y += 0.75f * cnst::playerHeight;
	glm::vec3 playerSecondPos(pos);
	playerSecondPos.y += 0.25f * cnst::playerHeight;

	if (glm::length(playerFirstPos) < 1.0f + cnst::sphereRadius)
		pos += glm::normalize(playerFirstPos) * (1.0f + cnst::sphereRadius - glm::length(playerFirstPos));
	else if (glm::length(playerSecondPos) < 1.0f + cnst::sphereRadius)
		pos += glm::normalize(playerSecondPos) * (1.0f + cnst::sphereRadius - glm::length(playerSecondPos));
}
