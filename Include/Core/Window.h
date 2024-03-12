#pragma once
#include "Ember.h"
#include "Renderer/ember_renderer_vulkan.h"

struct ember_window_settings_t
{
    string_t Title;
    u32 PosX;
    u32 PosY;
    s32 Width;
    s32 Height;
    u32 Flags;
};

struct ember_window_t
{
    SDL_Window* Handle;
    ember_renderer_vulkan_t Renderer;
    ember_window_settings_t Settings;
};

bool EmberWindowInit(ember_window_t* Window, ember_window_settings_t Settings);
void EmberWindowDestroy(ember_window_t* Window);
void EmberWindowBeginFrame(ember_window_t* Window);
void EmberWindowEndFrame(ember_window_t* Window);