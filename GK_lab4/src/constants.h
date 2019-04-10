#pragma once

#include <glm/glm.hpp>

namespace cnst
{
	const float screenMainPortionHeight = 0.7f;
	const float initOverviewCamDistance = 40.0f;
	const float overviewCamPitch = -30.0f;

	const unsigned int initWidth = 640;
	const unsigned int initHeight = 480;

	const char interfacePath[] = "resources/levels/interface.png";
	const char playerPath[] = "resources/levels/player.obj";
	const char cylinderPath[] = "resources/levels/cylinder.obj";
	const char spherePath[] = "resources/levels/sphere.obj";

	const char skyboxOrder[] = { 'r', 'l', 'u', 'd', 'f', 'b'};

	const float gravity = 6.0f;
	const float wallDistance = 0.1f;
	const float jumpVel = 9.0f;
	const float throwVel = 10.0f;
	const float objectFloatHeight = 1.5f;
	const float barrierHeight = -50.0f;

	const float baseMoveSpeed = 7.0f;
	const float maxMoveSpeed = 24.0f;
	const float walljumpThreshold = 12.0f;
	const float walljumpPenalty = 4.1f;
	const float acceleration = 1.0f;
	const float airControl = 1.0f;
	const glm::vec3 playerColor(0.863f, 0.078f, 0.235f);
	const float playerHeight = 3.0f;
	const glm::vec3 objectColColor(0.0f, 1.0f, 0.0f);

	const float objectRadius = 1.5f; // wlasciwie to srednica
	const float objectRotationSpeed = 30.0f;

	const float sphereRadius = 7.0f; // jeszcze troche zwiekszyc i rozbudowac poziom
	const glm::vec3 sphereColor(1.000f, 0.804f, 0.580f);

	const float platformTextureScale = 0.25f;
	const glm::vec3 clearColor(0.3f, 0.3f, 0.5f);

	const char instructions[] =
		"TRY PRESSING THESE\n"
		"    9/0/PLUS/MINUS\n"
		"           H/J/K/L\n"
		"           W/S/A/D\n"
		"             SPACE\n"
		"             R/ESC\n"
		"               F12\n"
		"               LMB\n"
		"               Q/E\n"
		"               I/O\n"
		"     P/ENTER/RCTRL\n"
		"OPEN/CLOSE BRACKET\n";

	const float baseTestSpeed1 = 20.0f;
	const float baseTestSpeed2 = 2.0f;
	const float sensitivitySpeed = 0.001f;

	const unsigned int maxLights = 64;

	const float cubeVertices[] =
	{
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	const float squareVertices[] =
	{
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
	};
	const unsigned int squareIndices[] =
	{  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	const float skyboxVertices[] =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	const unsigned char fallbackTexture[] = { 137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0,
		32, 0, 0, 0, 32, 8, 2, 0, 0, 0, 252, 24, 237, 163, 0, 0, 0, 9, 112, 72, 89, 115, 0, 0, 11, 19, 0, 0, 11, 19,
		1, 0, 154, 156, 24, 0, 0, 0, 7, 116, 73, 77, 69, 7, 227, 1, 19, 12, 12, 44, 180, 191, 200, 198, 0, 0, 0, 29,
		105, 84, 88, 116, 67, 111, 109, 109, 101, 110, 116, 0, 0, 0, 0, 0, 67, 114, 101, 97, 116, 101, 100, 32, 119,
		105, 116, 104, 32, 71, 73, 77, 80, 100, 46, 101, 7, 0, 0, 0, 112, 73, 68, 65, 84, 72, 75, 237, 145, 225, 10,
		0, 33, 8, 131, 187, 232, 253, 95, 249, 18, 2, 145, 177, 51, 162, 32, 56, 214, 175, 212, 165, 243, 171, 20,
		29, 17, 16, 1, 17, 184, 79, 224, 137, 22, 222, 16, 88, 97, 39, 228, 155, 197, 142, 166, 240, 112, 92, 150,
		66, 31, 80, 249, 168, 115, 217, 6, 173, 220, 230, 96, 247, 181, 19, 53, 0, 98, 162, 161, 237, 114, 50, 78,
		18, 48, 46, 32, 202, 127, 27, 108, 130, 216, 170, 19, 68, 100, 205, 144, 154, 51, 201, 223, 171, 42, 2, 34,
		32, 2, 191, 33, 208, 1, 142, 188, 31, 248, 80, 232, 22, 147, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };

	const char fallbackModel[] =
		"v -1.000000 -1.000000 1.000000\n"
		"v -1.000000 1.000000 1.000000\n"
		"v -1.000000 -1.000000 -1.000000\n"
		"v -1.000000 1.000000 -1.000000\n"
		"v 1.000000 -1.000000 1.000000\n"
		"v 1.000000 1.000000 1.000000\n"
		"v 1.000000 -1.000000 -1.000000\n"
		"v 1.000000 1.000000 -1.000000\n"
		"vt 0.000100 0.000100\n"
		"vt 0.999900 0.999900\n"
		"vt 0.000100 0.999900\n"
		"vt 0.000100 0.000100\n"
		"vt 0.999900 0.999900\n"
		"vt 0.000100 0.999900\n"
		"vt 0.999900 0.999900\n"
		"vt 0.000100 0.000100\n"
		"vt 0.999900 0.000100\n"
		"vt 0.999900 0.999900\n"
		"vt 0.000100 0.000100\n"
		"vt 0.999900 0.000100\n"
		"vt 0.000100 0.000100\n"
		"vt 0.000100 0.999900\n"
		"vt 0.999900 0.000100\n"
		"vt 0.999900 0.000100\n"
		"vt 0.999900 0.000100\n"
		"vt 0.000100 0.999900\n"
		"vt 0.000100 0.999900\n"
		"vt 0.999900 0.999900\n"
		"vn -1.0000 0.0000 0.0000\n"
		"vn 0.0000 0.0000 -1.0000\n"
		"vn 1.0000 0.0000 0.0000\n"
		"vn 0.0000 0.0000 1.0000\n"
		"vn 0.0000 -1.0000 0.0000\n"
		"vn 0.0000 1.0000 0.0000\n"
		"f 2/1/1 3/2/1 1/3/1\n"
		"f 4/4/2 7/5/2 3/6/2\n"
		"f 8/7/3 5/8/3 7/9/3\n"
		"f 6/10/4 1/11/4 5/12/4\n"
		"f 7/9/5 1/3/5 3/13/5\n"
		"f 4/14/6 6/15/6 8/7/6\n"
		"f 2/1/1 4/16/1 3/2/1\n"
		"f 4/4/2 8/17/2 7/5/2\n"
		"f 8/7/3 6/18/3 5/8/3\n"
		"f 6/10/4 2/19/4 1/11/4\n"
		"f 7/9/5 5/20/5 1/3/5\n"
		"f 4/14/6 2/1/6 6/15/6\n";

	const char fallbackUntexturedModel[] =
		"v -1.000000 -1.000000 1.000000\n"
		"v -1.000000 1.000000 1.000000\n"
		"v -1.000000 -1.000000 -1.000000\n"
		"v -1.000000 1.000000 -1.000000\n"
		"v 1.000000 -1.000000 1.000000\n"
		"v 1.000000 1.000000 1.000000\n"
		"v 1.000000 -1.000000 -1.000000\n"
		"v 1.000000 1.000000 -1.000000\n"
		"vn -0.5774 0.5774 0.5774\n"
		"vn -0.5774 -0.5774 -0.5774\n"
		"vn -0.5774 -0.5774 0.5774\n"
		"vn -0.5774 0.5774 -0.5774\n"
		"vn 0.5774 -0.5774 -0.5774\n"
		"vn 0.5774 0.5774 -0.5774\n"
		"vn 0.5774 -0.5774 0.5774\n"
		"vn 0.5774 0.5774 0.5774\n"
		"f 2//1 3//2 1//3\n"
		"f 4//4 7//5 3//2\n"
		"f 8//6 5//7 7//5\n"
		"f 6//8 1//3 5//7\n"
		"f 7//5 1//3 3//2\n"
		"f 4//4 6//8 8//6\n"
		"f 2//1 4//4 3//2\n"
		"f 4//4 8//6 7//5\n"
		"f 8//6 6//8 5//7\n"
		"f 6//8 2//1 1//3\n"
		"f 7//5 5//7 1//3\n"
		"f 4//4 2//1 6//8\n";

	const char fallbackLights[] =
		"amb 0.1 0.1 0.1\n"
		"sun 0.9 0.8 0.8 0.0 -1.0 0.0\n";

	const char fallbackLevel[] =
		"pltfrm -10.0 -10.0 -10.0 20.0 10.0 20.0 0\n"
		"pltfrm 5.0 5.0 5.0 5.0 5.0 5.0 0\n"
		"pltfrm -12.0 0.0 -6.0 6.0 6.0 6.0 0\n"
		"player 5.0 0.0 5.0 0.0\n"
		"object -5.0 2.0 5.0\n"
		"object -3.0 2.0 5.0\n"
		"object -1.0 2.0 5.0\n"
		"object 1.0 2.0 5.0\n"
		"object 3.0 2.0 5.0\n"
		"object 5.0 2.0 5.0\n"
		"object 7.0 2.0 5.0\n"
		"object 9.0 2.0 5.0\n"
		"camera 0.0 5.0 10.0\n";
}