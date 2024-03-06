#include "Core/Application.h"
#include "Core/Logging.h"

using namespace Ember;

Application::Application()
	: bIsRunning(false)
	, Window(nullptr)
{
}

bool Application::Init(const AppConfig& Config)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		EMBER_LOG(Critical, "SDL_Init Failure: %s", SDL_GetError());
		return false;
	}

	Window = Ember::MakeUnique<Ember::Window>(Config.WindowSettings);
	if (!Window)
	{
		EMBER_LOG(Critical, "Window Init Failure: %s", SDL_GetError());
		return false;
	}

	bIsRunning = Window->Init();
	return bIsRunning;
}

void Application::TearDown()
{
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
	{
		// TODO(HO): Rendering
	}
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
	if(Window)
	{
		Window->RenderBegin();
	}
}

void Application::RenderEnd()
{
	if(Window)
	{
		Window->RenderEnd();
	}
}
