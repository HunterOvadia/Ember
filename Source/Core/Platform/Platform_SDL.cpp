#include "Core/Platform/Platform.h"
#include "Core/Logging.h"
#include "Core/Application.h"
#include <SDL/SDL.h>
#include <Imgui/imgui_impl_sdl2.h>
#include <SDL/SDL_vulkan.h>

#include "Core/Renderer/Renderer_Vulkan.h"

// Platform Main
bool EmberPlatformInit(platform_t* Platform)
{
    UNUSED_ARG(Platform);
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        EMBER_LOG(Critical, "SDL_Init Failure: %s", SDL_GetError());
        return false;
    }
    
    return true;
}

void EmberPlatformShutdown(platform_t* Platform)
{
    UNUSED_ARG(Platform);
    SDL_Quit();
}

void EmberPlatformPollEvents(platform_t* Platform, ember_app_t* App)
{
    UNUSED_ARG(Platform);
    
    SDL_Event E;
    while (SDL_PollEvent(&E))
    {
        ImGui_ImplSDL2_ProcessEvent(&E);
        switch(E.type)
        {
            case SDL_QUIT:
            {
                App->State.IsRunning = false;
            }
            break;
            default:
            {
            }
            break;
        }
    }
}

// Platform Window
bool EmberPlatformCreateWindow(ember_window_settings_t WindowSettings, void** OutHandle)
{
    EMBER_ASSERT(OutHandle);
    
    *OutHandle = SDL_CreateWindow(WindowSettings.Title.CStr(), (int)WindowSettings.PosX, (int)WindowSettings.PosY, WindowSettings.Width, WindowSettings.Height, WindowSettings.Flags);
    return (*OutHandle != nullptr);
}

void EmberPlatformDestroyWindow(ember_window_t* Window)
{
    EMBER_ASSERT(Window);
    if(Window->Handle)
    {
        SDL_DestroyWindow((SDL_Window*)Window->Handle);
        Window->Handle = nullptr;
    }
}

bool EmberPlatformGetWindowSize(ember_window_t* Window, u32* OutWidth, u32* OutHeight)
{
    EMBER_ASSERT(Window && OutWidth && OutHeight);
    SDL_GetWindowSize((SDL_Window*)Window->Handle, (int*)OutWidth, (int*)OutHeight);
    return true;
}

// Platform ImGui
bool EmberPlatformImguiInit(ember_window_t* Window)
{
    return ImGui_ImplSDL2_InitForVulkan((SDL_Window*)Window->Handle);
}

void EmberPlatformImguiNewFrame()
{
    ImGui_ImplSDL2_NewFrame();
}

void EmberPlatformImguiShutdown()
{
    ImGui_ImplSDL2_Shutdown();
}

// Platform Renderer - Vulkan
DynamicArray<const char*> EmberPlatformRendererVulkanGetInstanceExtensions(ember_window_t* Window)
{
    u32 ExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions((SDL_Window*)Window->Handle, &ExtensionsCount, nullptr);

    DynamicArray<const char*> Extensions(ExtensionsCount);
    SDL_Vulkan_GetInstanceExtensions((SDL_Window*)Window->Handle, &ExtensionsCount, Extensions.GetData());
    return Extensions;
}

bool EmberPlatformRendererCreateVulkanSurface(ember_renderer_vulkan_t* Renderer, ember_window_t* Window)
{
    return SDL_Vulkan_CreateSurface((SDL_Window*)Window->Handle, Renderer->Context.Instance, &Renderer->Context.Surface);
}
