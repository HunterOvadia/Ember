#pragma once
#include "Ember.h"

struct AppConfig
{
	const char* WindowTitle;
	int WindowInitPosX;
	int WindowInitPosY;
	int WindowInitWidth;
	int WindowInitHeight;
	u32 WindowFlags;

	static AppConfig GetDefault()
	{
		static AppConfig Result =
		{
			.WindowTitle = "New Window",
			.WindowInitPosX = SDL_WINDOWPOS_CENTERED,
			.WindowInitPosY = SDL_WINDOWPOS_CENTERED,
			.WindowInitWidth = 800,
			.WindowInitHeight = 600,
			.WindowFlags = SDL_WINDOW_SHOWN
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
	
	bool Init(const AppConfig& Config);
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