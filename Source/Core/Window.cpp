#include "Core/Window.h"
#include "Core/Logging.h"
#include "Core/Platform/Platform.h"
#include "Core/Renderer/Renderer_Vulkan.h"

bool EmberWindowInit(ember_window_t* Window, ember_window_settings_t WindowSettings)
{
	if(!EmberPlatformCreateWindow(WindowSettings, &Window->Handle))
	{
		EMBER_LOG(Critical, "EmberPlatformCreateWindow Failure: %s");
		return false;
	}

	EMBER_LOG(Info, "Window Initialize Success.");
	return true;
}

void EmberWindowDestroy(ember_window_t* Window)
{
	EmberPlatformDestroyWindow(Window);
}