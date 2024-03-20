#pragma once

struct ember_renderer_t;
struct ember_window_t;

bool ImGuiInit(ember_renderer_t* Renderer, ember_window_t* Window);
void ImGuiShutdown();
void ImGuiNewFrame();
void ImGuiEndFrame();