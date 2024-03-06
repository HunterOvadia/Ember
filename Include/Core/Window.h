#pragma once
#include "Ember.h"
#include <SDL/SDL_render.h>

struct WindowSettings
{
    const char* Title;
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
            .Width = 800,
            .Height = 600,
            .Flags = SDL_WINDOW_SHOWN
        };

        return Result;
    }
};

namespace Ember
{
    class Window
    {
    public:
        explicit Window(const WindowSettings& Settings)
            : SDLWindow(nullptr)
            , SDLRenderer(nullptr)
        {
            this->Settings = Settings;
        }

        ~Window()
        {
           TearDown();
        }

        SDL_Window* GetWindow() const { return SDLWindow; }
        SDL_Renderer* GetRenderer() const { return SDLRenderer; }

        bool Init();
        void TearDown();
        
        void RenderBegin();
        void RenderEnd();
        
    private:
        SDL_Window* SDLWindow;
        SDL_Renderer* SDLRenderer;
        WindowSettings Settings;
    };
}

