#include "Ember.h"
#include "Core/Application.h"

int main(int argc, char** argv)
{
	Application App;
	
	AppConfig Config = AppConfig::GetDefault();
	Config.WindowSettings.Title = "Ember";
	if (App.Init(Config))
	{
		App.Run();
	}

	App.TearDown();
	return 0;
}