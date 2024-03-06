#pragma once
#include "Ember.h"

struct ApplicationConfiguration
{
	const char* Title;
	int X;
	int Y;
	int Width;
	int Height;
	u32 Flags;

	static ApplicationConfiguration GetDefault()
	{
		static ApplicationConfiguration Result =
		{
			.Title = "New Window",
			.X = SDL_WINDOWPOS_CENTERED,
			.Y = SDL_WINDOWPOS_CENTERED,
			.Width = 800,
			.Height = 600,
			.Flags = SDL_WINDOW_SHOWN
		};

		return Result;
	}
};

class Application
{
public:
	Application();
	~Application() = default;
	Application(Application& Other) = delete;
	Application(Application&& Other) = delete;
	
	bool Init(const ApplicationConfiguration& Configuration);
	void Shutdown();
	void Run();

private:
	void Update();
	void Render();
	void PollEvents();

	void RenderBegin();
	void RenderEnd();

private:
	bool bIsRunning;
	SDL_Window* Window;
	SDL_Renderer* Renderer;
};