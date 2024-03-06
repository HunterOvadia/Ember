#include "Ember.h"
#include "Core/Application.h"

int main(int argc, char** argv)
{
	Application App;
	AppConfig Config = AppConfig::GetDefault();
	Config.WindowTitle = "Ember";
	if (App.Init(Config))
	{
		App.Run();
	}

	App.Shutdown();
	return 0;
}