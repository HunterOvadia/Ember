#pragma once
#include "Ember.h"
#include "Renderer/VulkanRenderer.h"

namespace Ember
{
    struct WindowSettings
    {
        String Title;
        int PosX;
        int PosY;
        int Width;
        int Height;
        u32 Flags;

        static WindowSettings GetDefault()
        {
            static WindowSettings Result =
            {
                .Title = "New Window",
                .PosX = SDL_WINDOWPOS_CENTERED,
                .PosY = SDL_WINDOWPOS_CENTERED,
                .Width = 1280,
                .Height = 720,
                .Flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
            };

            return Result;
        }
    };
    
    class Window
    {
    public:
        explicit Window(const WindowSettings& Settings)
            : SDLWindow(nullptr)
            , Renderer(nullptr)
            , Settings(Settings)
        {
        }

        SDL_Window* Get() const { return SDLWindow; }

        bool Init();
        void TearDown();
        void RenderBegin();
        void RenderEnd();
        
    private:
        SDL_Window* SDLWindow;
        VulkanRenderer* Renderer;
        WindowSettings Settings;
    };
}

