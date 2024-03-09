#pragma once
#include "Window.h"

namespace Ember
{
	struct AppConfig
	{
		WindowSettings WindowSettings;
		static AppConfig GetDefault(const String& TitleOverride = "")
		{
			static AppConfig Result =
			{
				.WindowSettings = WindowSettings::GetDefault()
			};

			if(TitleOverride.GetLength() > 0)
			{
				Result.WindowSettings.Title = TitleOverride.CStr();
			}

			return Result;
		}
	};

	class Application
	{
	public:
		Application();
		~Application() = default;
		Application(const Application& Other) = delete;
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
