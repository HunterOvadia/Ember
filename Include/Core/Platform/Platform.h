#pragma once
#include "Ember.h"
#include "Containers/DynamicArray.h"

struct ember_app_t;
struct ember_window_t;
struct ember_window_settings_t;
struct ember_window_t;
struct ember_renderer_vulkan_t;

struct platform_t
{
    void* Internal;
};

// Platform Main
bool EmberPlatformInit(platform_t* Platform);
void EmberPlatformShutdown(platform_t* Platform);
void EmberPlatformPollEvents(platform_t* Platform, ember_app_t* App);

// Platform Window
bool EmberPlatformCreateWindow(ember_window_settings_t WindowSettings, void** OutHandle);
void EmberPlatformDestroyWindow(ember_window_t* Window);
bool EmberPlatformGetWindowSize(ember_window_t* Window, u32* OutWidth, u32* OutHeight);

// Platform ImGui
bool EmberPlatformImguiInit(ember_window_t* Window);
void EmberPlatformImguiNewFrame();
void EmberPlatformImguiShutdown();

// Platform Renderer - Vulkan
DynamicArray<const char*> EmberPlatformRendererVulkanGetInstanceExtensions(ember_window_t* Window);
bool EmberPlatformRendererCreateVulkanSurface(ember_renderer_vulkan_t* Renderer, ember_window_t* Window);
