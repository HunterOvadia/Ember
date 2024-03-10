#include "Core/Window.h"
#include "Core/Logging.h"
#include "Imgui/imgui.h"
using namespace Ember;

bool Window::Init()
{
    SDLWindow = SDL_CreateWindow(Settings.Title.CStr(), Settings.PosX, Settings.PosY, Settings.Width, Settings.Height, Settings.Flags);
    if(!SDLWindow)
    {
        EMBER_LOG(Critical, "SDL_CreateWindow Failure: %s", SDL_GetError());
        return false;
    }

	Renderer = new VulkanRenderer(this);
	if(!Renderer || !Renderer->Initialize())
	{
		EMBER_LOG(Critical, "VulkanRenderer Create Failure!");
		return false;
	}
	
    EMBER_LOG(Info, "Window Initialize Success.");
    return true;
}

void Window::TearDown()
{
	if(Renderer)
	{
		Renderer->Shutdown();
	}
	
    if(SDLWindow)
    {
        SDL_DestroyWindow(SDLWindow);
        SDLWindow = nullptr;
    }
}

void Window::RenderBegin()
{
	if(Renderer)
	{
		Renderer->BeginFrame();
	}
}

void Window::RenderEnd()
{
	if(Renderer)
	{
		Renderer->EndFrame();
	}
}
