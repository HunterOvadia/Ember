#include <SDL/SDL_video.h>
#include "Core/Application.h"

int main(int ArgC, char** ArgV)
{
	UNUSED_ARG(ArgC);
	UNUSED_ARG(ArgV);

	ember_app_t App = {};
	ember_app_config_t Config = {
		.WindowSettings = {
			.Title = "Ember Editor",
			.PosX = SDL_WINDOWPOS_CENTERED,
			.PosY = SDL_WINDOWPOS_CENTERED,
			.Width = 1280,
			.Height = 720,
			.Flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
		}
	};

	if(EmberAppInit(&App, Config))
	{
		EmberAppRun(&App);
	}

	EmberAppDestroy(&App);
	return 0;
}