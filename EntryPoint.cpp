#include "Ember.h"
#include "Application.h"

int main(int argc, char** argv)
{
	Application App;
	if (App.Init())
	{
		App.Run();
	}

	App.Shutdown();
	return 0;
}