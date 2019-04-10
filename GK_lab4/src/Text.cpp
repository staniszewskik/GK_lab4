#include "Text.h"

#include "utility.h"
#include "constants.h"

Text::Text(unsigned int progText, unsigned int width, unsigned int height)
{
	interfaceTextureID = util::createInterfaceTexture();

	progTextShader = progText;

	unsigned int cursorY = (unsigned int)(cnst::screenMainPortionHeight * height / 2) - 8;
	unsigned int cursorX = width / 2 - 8;

	cursorVertSize[0] = 24 * sizeof(float);
	cursorVert[0] = new float[24];
	copySquare(cursorVert[0], 1, 3, cursorX, cursorY, width, (unsigned int)(cnst::screenMainPortionHeight * height));
	util::bindVAO_Text(&cursorVAO[0], &cursorVBO[0], cursorVert[0], cursorVertSize[0]);

	cursorVertSize[1] = 24 * sizeof(float);
	cursorVert[1] = new float[24];
	copySquare(cursorVert[1], 2, 3, cursorX, cursorY, width, (unsigned int)(cnst::screenMainPortionHeight * height));
	util::bindVAO_Text(&cursorVAO[1], &cursorVBO[1], cursorVert[1], cursorVertSize[1]);
}

Text::~Text()
{
	glDeleteTextures(1, &interfaceTextureID);

	delete[] cursorVert[0];
	delete[] cursorVert[1];

	glDeleteVertexArrays(1, &cursorVAO[0]);
	glDeleteVertexArrays(1, &cursorVAO[1]);
	glDeleteBuffers(1, &cursorVBO[0]);
	glDeleteBuffers(1, &cursorVBO[1]);
}

void Text::drawText(std::string text, unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool wF)
{
	struct square
	{
		unsigned int xTex, yTex;
		unsigned int xTgt, yTgt;
	};
	std::vector<square> squares;
	square current;
	current.xTgt = x;
	current.yTgt = y;

	for (unsigned int i = 0; i < text.size(); i++)
	{
		if (text[i] == ' ')
			current.xTgt += 16;
		else if (text[i] == '\n')
		{
			current.xTgt = x;
			current.yTgt -= 16;
		}
		else if (text[i] == '/')
		{
			current.xTex = 0;
			current.yTex = 3;
			squares.push_back(current);
			current.xTgt += 16;
		}
		else if (text[i] >= 'A' && text[i] <= 'Z')
		{
			current.xTex = (text[i] - 'A') % 16;
			current.yTex = (text[i] - 'A') / 16;
			squares.push_back(current);
			current.xTgt += 16;
		}
		else if (text[i] >= '0' && text[i] <= '9')
		{
			current.xTex = text[i] - '0';
			current.yTex = 2;
			squares.push_back(current);
			current.xTgt += 16;
		}
	}

	unsigned int textVertSize = 24 * sizeof(float) * squares.size();
	float *textVert = new float[textVertSize / sizeof(float)];
	for (unsigned int i = 0; i < squares.size(); i++)
		copySquare(textVert + i * 24, squares[i].xTex, squares[i].yTex, squares[i].xTgt, squares[i].yTgt, w, h);

	unsigned int textVAO, textVBO;
	util::bindVAO_Text(&textVAO, &textVBO, textVert, textVertSize);

	if (wF)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, interfaceTextureID);

	glUseProgram(progTextShader);
	glUniform1i(glGetUniformLocation(progTextShader, "interfaceTexture"), 2);

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, textVertSize / (sizeof(float) * 4));

	glEnable(GL_DEPTH_TEST);

	if (wF)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	delete[] textVert;

	glDeleteVertexArrays(1, &textVAO);
	glDeleteBuffers(1, &textVBO);
}

void Text::drawCursor(bool walljump, bool wF)
{
	if (wF)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, interfaceTextureID);

	glUseProgram(progTextShader);
	glUniform1i(glGetUniformLocation(progTextShader, "interfaceTexture"), 2);

	glBindVertexArray(cursorVAO[walljump]);
	glDrawArrays(GL_TRIANGLES, 0, cursorVertSize[walljump] / (sizeof(float) * 4));

	glEnable(GL_DEPTH_TEST);

	if (wF)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Text::copySquare(float *vert, unsigned int xTex, unsigned int yTex, unsigned int xTgt, unsigned int yTgt,
	unsigned int wTgt, unsigned int hTgt)
{
	// trojkat 1
	vert[0 * 4 + 0] = (float)xTgt / wTgt * 2.0f - 1.0f;
	vert[0 * 4 + 1] = (float)yTgt / hTgt * 2.0f - 1.0f;

	vert[0 * 4 + 2] = (float)xTex / 16.0f;
	vert[0 * 4 + 3] = (float)(16 - (yTex + 1)) / 16.0f;

	vert[1 * 4 + 0] = (float)(xTgt + 16) / wTgt * 2.0f - 1.0f;
	vert[1 * 4 + 1] = (float)(yTgt + 16) / hTgt * 2.0f - 1.0f;

	vert[1 * 4 + 2] = (float)(xTex + 1) / 16.0f;
	vert[1 * 4 + 3] = (float)(16 - (yTex + 1) + 1) / 16.0f;

	vert[2 * 4 + 0] = (float)xTgt / wTgt * 2.0f - 1.0f;
	vert[2 * 4 + 1] = (float)(yTgt + 16) / hTgt * 2.0f - 1.0f;

	vert[2 * 4 + 2] = (float)xTex / 16.0f;
	vert[2 * 4 + 3] = (float)(16 - (yTex + 1) + 1) / 16.0f;

	// trojkat 2
	vert[3 * 4 + 0] = vert[0 * 4 + 0];
	vert[3 * 4 + 1] = vert[0 * 4 + 1];

	vert[3 * 4 + 2] = vert[0 * 4 + 2];
	vert[3 * 4 + 3] = vert[0 * 4 + 3];

	vert[4 * 4 + 0] = (float)(xTgt + 16) / wTgt * 2.0f - 1.0f;
	vert[4 * 4 + 1] = (float)yTgt / hTgt * 2.0f - 1.0f;

	vert[4 * 4 + 2] = (float)(xTex + 1) / 16.0f;
	vert[4 * 4 + 3] = (float)(16 - (yTex + 1)) / 16.0f;

	vert[5 * 4 + 0] = vert[1 * 4 + 0];
	vert[5 * 4 + 1] = vert[1 * 4 + 1];

	vert[5 * 4 + 2] = vert[1 * 4 + 2];
	vert[5 * 4 + 3] = vert[1 * 4 + 3];
}
