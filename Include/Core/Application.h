#pragma once
#include "Window.h"

struct ember_app_config_t
{
	ember_window_settings_t WindowSettings;
};

struct ember_app_t
{
	ember_window_t Window;
	bool bIsRunning;
};

bool EmberAppInit(ember_app_t* App, ember_app_config_t Config);
void EmberAppDestroy(ember_app_t* App);
void EmberAppRun(ember_app_t* App);