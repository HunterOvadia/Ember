#pragma once
struct ember_window_t;

struct ember_renderer_t
{
    void* Internal;
};

// Main
bool EmberRendererInit(ember_renderer_t* Renderer, ember_window_t* Window);
void EmberRendererShutdown(ember_renderer_t* Renderer);
void EmberRendererBeginFrame(ember_renderer_t* Renderer, ember_window_t* Window);
void EmberRendererEndFrame(ember_renderer_t* Renderer);

// ImGui
bool EmberRendererImGuiInit(ember_renderer_t* Renderer);
void EmberRendererImGuiShutdown();
