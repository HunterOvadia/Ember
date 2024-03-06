#include "Ember.h"
#include "Application.h"

int main(int argc, char** argv)
{
	Application App;
	
	ApplicationConfiguration Configuration = ApplicationConfiguration::GetDefault();
	Configuration.Title = "Ember";
	if (App.Init(Configuration))
	{
		App.Run();
	}

	App.Shutdown();
	return 0;
}