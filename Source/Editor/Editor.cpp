#include "Editor/Editor.h"
#include <Imgui/imgui.h>

void EmberEditorRender()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    EditorMenuBarRender();
}
