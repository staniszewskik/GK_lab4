#include "ResMan.h"

#include <ios>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem> 
using namespace std::experimental;

#include "constants.h"

ResMan::ResMan()
{
	if (!filesystem::exists("resources"))
		filesystem::create_directory("resources");

	if (!filesystem::exists("resources/models"))
		filesystem::create_directory("resources/models");
	if (!filesystem::exists("resources/levels"))
		filesystem::create_directory("resources/levels");
	if (!filesystem::exists("resources/lights"))
		filesystem::create_directory("resources/lights");
	if (!filesystem::exists("resources/textures"))
		filesystem::create_directory("resources/textures");

	// domyslny konstruktor tworzy iterator koncowy
	filesystem::directory_iterator end;

	filesystem::directory_iterator modelIter("resources/models");
	while (modelIter != end)
	{
		if(modelIter->path().extension().string() == ".obj")
			modelNames.push_back(modelIter->path().stem().string());
		modelIter++;
	}
	if (modelNames.size() == 0)
	{
		std::ofstream objout("resources/models/fallback_model.obj", std::ios::out);
		objout << cnst::fallbackModel;
		objout.close();

		modelNames.push_back("fallback_model");
	}

	filesystem::directory_iterator levelIter("resources/levels");
	while (levelIter != end)
	{
		if(levelIter->path().extension().string() == ".levelgeo")
			levelNames.push_back(levelIter->path().stem().string());
		levelIter++;
	}
	if (levelNames.size() == 0)
	{
		std::ofstream levout("resources/levels/fallback_level.levelgeo", std::ios::out);
		levout << cnst::fallbackLevel;
		levout.close();

		levelNames.push_back("fallback_level");
	}

	// najwyrazniej porzadek alfabetyczny nie jest gwarantowany, sortujemy znalezione nazwy
	// (bo np. na kolejnosci zaladowania modeli polega ich pozycja w poziomie)
	std::sort(modelNames.begin(), modelNames.end());
	std::sort(levelNames.begin(), levelNames.end());

	for (unsigned int i = 0; i < modelNames.size(); i++)
	{
		if (!filesystem::exists("resources/lights/" + modelNames[i] + ".lightdat"))
		{
			std::ofstream lgtout("resources/lights/" + modelNames[i] + ".lightdat", std::ios::out);
			lgtout << cnst::fallbackLights;
			lgtout.close();
		}

		if (!filesystem::exists("resources/textures/" + modelNames[i] + ".png"))
		{
			std::ofstream texout("resources/textures/" + modelNames[i] + ".png", std::ios::out | std::ios::binary);
			texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
			texout.close();
		}
	}

	for (unsigned int i = 0; i < levelNames.size(); i++)
	{
		if (!filesystem::exists("resources/lights/" + levelNames[i] + ".lightdat"))
		{
			std::ofstream lgtout("resources/lights/" + levelNames[i] + ".lightdat", std::ios::out);
			lgtout << cnst::fallbackLights;
			lgtout.close();
		}

		for (int j = 0; j < 3; j++)
		{
			if (!filesystem::exists("resources/textures/" + levelNames[i] + "_" + (char)(j + '0') + ".png"))
			{
				std::ofstream texout("resources/textures/" + levelNames[i] + "_" + (char)(j + '0') + ".png",
					std::ios::out | std::ios::binary);
				texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
				texout.close();
			}
		}

		for (int j = 0; j < 6; j++)
		{
			if (!filesystem::exists("resources/textures/" + levelNames[i] + "_" + cnst::skyboxOrder[j] + ".png"))
			{
				std::ofstream texout("resources/textures/" + levelNames[i] + "_" + cnst::skyboxOrder[j] + ".png",
					std::ios::out | std::ios::binary);
				texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
				texout.close();
			}
		}
	}

	if (!filesystem::exists(cnst::interfacePath))
	{
		std::ofstream texout(cnst::interfacePath, std::ios::out | std::ios::binary);
		texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
		texout.close();
	}

	if (!filesystem::exists(cnst::playerPath))
	{
		std::ofstream objout(cnst::playerPath, std::ios::out);
		objout << cnst::fallbackUntexturedModel;
		objout.close();
	}

	if (!filesystem::exists(cnst::spherePath))
	{
		std::ofstream objout(cnst::spherePath, std::ios::out);
		objout << cnst::fallbackUntexturedModel;
		objout.close();
	}

	if (!filesystem::exists(cnst::cylinderPath))
	{
		std::ofstream objout(cnst::cylinderPath, std::ios::out);
		objout << cnst::fallbackUntexturedModel;
		objout.close();
	}
}
