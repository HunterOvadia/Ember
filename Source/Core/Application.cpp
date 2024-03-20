#include "Core/Application.h"
#include "Core/Logging.h"
#include "Editor/EditorMenuBar.h"
#include "Imgui/imgui.h"

static void EmberAppUpdate(ember_app_t* App)
{
    // TODO(HO)
}

static void EmberAppRender(ember_app_t* App)
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    EditorMenuBarRender(App);
}

static void UpdateFrame(ember_app_t* App)
{
    EmberPlatformPollEvents(&App->Platform, App);
    EmberAppUpdate(App);
}

static void RenderFrame(ember_app_t* App)
{
    EmberWindowBeginFrame(&App->Window);
    {
        EmberAppRender(App);
    }
    EmberWindowEndFrame(&App->Window);
}

bool EmberAppInit(ember_app_t* App, ember_app_config_t Config)
{
    if(!EmberPlatformInit(&App->Platform))
    {
        EMBER_LOG(Critical, "Platform Init Failure.");
        return false;
    }
    
    if(!EmberWindowInit(&App->Window, Config.WindowSettings))
    {
        EMBER_LOG(Critical, "Window Init Failure.");
        return false;
    }
    
    App->State.IsRunning = true;
    return App->State.IsRunning;
}

void EmberAppDestroy(ember_app_t* App)
{
    EmberWindowDestroy(&App->Window);
    EmberPlatformShutdown(&App->Platform);
}

void EmberAppRun(ember_app_t* App)
{
    while(App->State.IsRunning)
    {
        UpdateFrame(App);
        RenderFrame(App);
    }
}

