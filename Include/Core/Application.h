#pragma once
#include "Window.h"

namespace Ember
{
	struct AppConfig
	{
		WindowSettings WindowSettings;
		static AppConfig GetDefault(const char* TitleOverride = nullptr)
		{
			static AppConfig Result =
			{
				.WindowSettings = WindowSettings::GetDefault()
			};

			if(TitleOverride)
			{
				Result.WindowSettings.Title = TitleOverride;
			}
			
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
		Ember::UniquePtr<Ember::Window> Window;
	};
}
