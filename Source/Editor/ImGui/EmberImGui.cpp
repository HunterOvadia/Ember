#include "Ember.h"
#include "Editor/ImGui/EmberImGui.h"
#include "Core/Platform/Platform.h"
#include "Core/Renderer/Renderer.h"
#include <Imgui/imgui.h>

bool EmberImGuiInit(ember_renderer_t* Renderer, ember_window_t* Window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& Io = ImGui::GetIO(); (void)Io;
    Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    Io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    Io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGuiStyle& Style = ImGui::GetStyle();
    if(Io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        Style.WindowRounding = 0.0f;
        Style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    bool Result = EmberPlatformImguiInit(Window);
    if(Result)
    {
        Result = EmberRendererImGuiInit(Renderer);
    }

    return Result;
}

void EmberImGuiShutdown()
{
    EmberRendererImGuiShutdown();
    EmberPlatformImguiShutdown();
    ImGui::DestroyContext();
}

void EmberImGuiNewFrame()
{
    EmberPlatformImguiNewFrame();
    ImGui::NewFrame();
}

void EmberImGuiEndFrame()
{
    ImGui::Render();
}
