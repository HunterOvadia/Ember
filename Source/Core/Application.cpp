#include "Core/Application.h"
#include <Imgui/imgui_impl_sdl2.h>
#include "Core/Logging.h"

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
                App->bIsRunning = false;
            }
            break;
            default:
            {
            }
            break;
        }
    }
}

static void UpdateFrame(ember_app_t* App)
{
    PollEvents(App);
    // Game Update
}

static void RenderFrame(ember_app_t* App)
{
    EmberWindowBeginFrame(&App->Window);
    {
        // Game Render
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

    App->bIsRunning = true;
    return App->bIsRunning;
}

void EmberAppDestroy(ember_app_t* App)
{
    EmberWindowDestroy(&App->Window);
    SDL_Quit();
}

void EmberAppRun(ember_app_t* App)
{
    while(App->bIsRunning)
    {
        UpdateFrame(App);
        RenderFrame(App);
    }
}