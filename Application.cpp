#include "Application.h"

Application::Application()
	: bIsRunning(false)
	, Window(nullptr)
	, Renderer(nullptr)
{
}

bool Application::Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		EMBER_LOG(Critical, "SDL_Init Failure: %s", SDL_GetError());
		return false;
	}

	// TODO(HO): Variables
	Uint32 WindowFlags = SDL_WINDOW_SHOWN;
	Window = SDL_CreateWindow("Ember", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, WindowFlags);
	if (!Window)
	{
		EMBER_LOG(Critical, "SDL_CreateWindow Failure: %s", SDL_GetError());
		return false;
	}

	Uint32 RendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	Renderer = SDL_CreateRenderer(Window, -1, RendererFlags);
	if (!Renderer)
	{
		EMBER_LOG(Critical, "SDL_CreateRenderer Failure: %s", SDL_GetError());
		return false;
	}
	
	bIsRunning = true;
	return true;
}

void Application::Shutdown()
{
	if (Renderer)
	{
		SDL_DestroyRenderer(Renderer);
		Renderer = nullptr;
	}

	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = nullptr;
	}

	SDL_Quit();
}

void Application::Run()
{
	while (bIsRunning)
	{
		Update();
		Render();
	}
}

void Application::Update()
{
	PollEvents();
}

void Application::Render()
{
	RenderBegin();
	// TODO(HO): Rendering
	RenderEnd();
}

void Application::PollEvents()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		switch (Event.type)
		{
			case SDL_QUIT:
			{
				bIsRunning = false;
			}
			break;

			default: {} break;
		}
	}
}

void Application::RenderBegin()
{
	SDL_RenderClear(Renderer);
}

void Application::RenderEnd()
{
	SDL_RenderPresent(Renderer);
}
