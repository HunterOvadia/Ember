#pragma once
#include <memory>
#include "Window.h"

struct AppConfig
{
	WindowSettings WindowSettings;
	static AppConfig GetDefault()
	{
		static AppConfig Result =
		{
			.WindowSettings = WindowSettings::GetDefault()
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
	void TearDown();
	void Run();

private:
	void Update();
	void Render();
	void PollEvents();

	void RenderBegin();
	void RenderEnd();

private:
	bool bIsRunning;
	std::unique_ptr<Ember::Window> Window;
};