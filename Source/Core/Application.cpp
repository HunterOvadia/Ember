#include "Core/Application.h"
#include <Imgui/imgui_impl_sdl2.h>
#include "Core/Logging.h"
#include "Editor/EditorMenuBar.h"

static void PollEvents(ember_app_t* App)
{
    SDL_Event E;
    while (SDL_PollEvent(&E))
    {
        ImGui_ImplSDL2_ProcessEvent(&E);
        switch(E.type)
        {
            case SDL_QUIT:
            {
                App->State.IsRunning = false;
            }
            break;
            default:
            {
            }
            break;
        }
    }
}

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
    PollEvents(App);
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        EMBER_LOG(Critical, "SDL_Init Failure: %s", SDL_GetError());
        return false;
    }

    if(!EmberWindowInit(&App->Window, Config.WindowSettings))
    {
        EMBER_LOG(Critical, "Window Init Failure: %s", SDL_GetError());
        return false;
    }
    
    App->State.IsRunning = true;
    return App->State.IsRunning;
}

void EmberAppDestroy(ember_app_t* App)
{
    EmberWindowDestroy(&App->Window);
    SDL_Quit();
}

void EmberAppRun(ember_app_t* App)
{
    while(App->State.IsRunning)
    {
        UpdateFrame(App);
        RenderFrame(App);
    }
}

