#pragma once
#include "Ember.h"

class Application
{
public:
	Application();
	~Application() = default;
	Application(Application& Other) = delete;
	Application(Application&& Other) = delete;
	
	bool Init();
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