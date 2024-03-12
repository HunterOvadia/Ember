﻿#include "Core/Window.h"
#include "Core/Logging.h"
#include "Core/Renderer/ember_renderer_vulkan.h"

bool EmberWindowInit(ember_window_t* Window, ember_window_settings_t Settings)
{
	Window->Settings = Settings;
	Window->Handle = SDL_CreateWindow(Settings.Title.CStr(), (int)Settings.PosX, (int)Settings.PosY, Settings.Width, Settings.Height, Settings.Flags);
	if(!Window->Handle)
	{
	    EMBER_LOG(Critical, "SDL_CreateWindow Failure: %s", SDL_GetError());
	    return false;
	}

	if(!EmberRendererInit(&Window->Renderer, Window))
	{
		EMBER_LOG(Critical, "VulkanRenderer Create Failure!");
		return false;
	}

	EMBER_LOG(Info, "Window Initialize Success.");
	return true;
}

void EmberWindowDestroy(ember_window_t* Window)
{
	EmberRendererShutdown(&Window->Renderer);
	if(Window->Handle)
	{
	    SDL_DestroyWindow(Window->Handle);
	    Window->Handle = nullptr;
	}
}

void EmberWindowBeginFrame(ember_window_t* Window)
{
	EmberRendererBeginFrame(&Window->Renderer);
}

void EmberWindowEndFrame(ember_window_t* Window)
{
	EmberRendererEndFrame(&Window->Renderer);
}