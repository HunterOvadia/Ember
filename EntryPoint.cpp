#include "Core/Application.h"

int main(int ArgC, char** ArgV)
{
	UNUSED_ARG(ArgC);
	UNUSED_ARG(ArgV);

	ember_app_t App = {};
	ember_app_config_t Config = {};
	Config.WindowSettings.Title = "Ember Editor";
	Config.WindowSettings.PosX = SDL_WINDOWPOS_CENTERED;
	Config.WindowSettings.PosY = SDL_WINDOWPOS_CENTERED;
	Config.WindowSettings.Width = 1280;
	Config.WindowSettings.Height = 720;
	Config.WindowSettings.Flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	if(EmberAppInit(&App, Config))
	{
		EmberAppRun(&App);
	}

	EmberAppDestroy(&App);
	return 0;
}