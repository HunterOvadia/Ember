#include "Core/Application.h"

int main(int ArgC, char** ArgV)
{
	UNUSED_ARG(ArgC);
	UNUSED_ARG(ArgV);
	
	Ember::Application App;
	Ember::AppConfig Config = Ember::AppConfig::GetDefault();
	Config.WindowSettings.Title = "Ember";
	if (App.Init(Config))
	{
		App.Run();
	}

	App.TearDown();
	return 0;
}