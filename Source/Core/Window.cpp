#include "Core/Window.h"
#include "Core/Logging.h"

using namespace Ember;

bool Window::Init()
{
    SDLWindow = SDL_CreateWindow(Settings.Title.CStr(), Settings.PosX, Settings.PosY, Settings.Width, Settings.Height, Settings.Flags);
    if(!SDLWindow)
    {
        EMBER_LOG(Critical, "SDL_CreateWindow Failure: %s", SDL_GetError());
        return false;
    }

    u32 RendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDLRenderer = SDL_CreateRenderer(SDLWindow, -1, RendererFlags);
    if (!SDLRenderer)
    {
        EMBER_LOG(Critical, "SDL_CreateRenderer Failure: %s", SDL_GetError());
        SDL_DestroyWindow(SDLWindow);
        SDLWindow = nullptr;
        return false;
    }

    EMBER_LOG(Info, "Window Initialize Success.");
    return true;
}

void Window::TearDown()
{
    if(SDLRenderer)
    {
        SDL_DestroyRenderer(SDLRenderer);
        SDLRenderer = nullptr;
    }
    
    if(SDLWindow)
    {
        SDL_DestroyWindow(SDLWindow);
        SDLWindow = nullptr;
    }
}

void Window::RenderBegin()
{
    SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 0, 255);
    SDL_RenderClear(SDLRenderer);
}

void Window::RenderEnd()
{
    SDL_RenderPresent(SDLRenderer);
}
