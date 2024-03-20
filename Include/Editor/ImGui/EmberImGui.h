#pragma once

struct ember_renderer_t;
struct ember_window_t;

bool EmberImGuiInit(ember_renderer_t* Renderer, ember_window_t* Window);
void EmberImGuiShutdown();
void EmberImGuiNewFrame();
void EmberImGuiEndFrame();