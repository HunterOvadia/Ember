#pragma once
#include "Window.h"

struct ember_app_config_t
{
	ember_window_settings_t WindowSettings;
};

struct app_state_t
{
	bool IsRunning;
};

struct ember_app_t
{
	ember_window_t Window;
	app_state_t State;
};

bool EmberAppInit(ember_app_t* App, ember_app_config_t Config);
void EmberAppDestroy(ember_app_t* App);
void EmberAppRun(ember_app_t* App);