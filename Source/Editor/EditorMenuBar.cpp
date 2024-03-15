#include "Editor/EditorMenuBar.h"

#include <Imgui/imgui.h>

void EditorMenuBarRender(ember_app_t* App)
{
    static editor_menu_bar_t MenuBar;
    
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New ..."))
            {
                
            }

            if(ImGui::MenuItem("Open"))
            {
                
            }

            if(ImGui::MenuItem("Save All"))
            {
                
            }
            
            ImGui::Separator();
            
            if(ImGui::MenuItem("Exit"))
            {
                
            }
            
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help"))
        {
            if(ImGui::MenuItem("Help"))
            {
                
            }

            if(ImGui::MenuItem("Tutorials"))
            {
                
            }

            ImGui::Separator();
            
            if(ImGui::MenuItem("Contact Support"))
            {
                
            }

            if(ImGui::MenuItem("Submit Feedback"))
            {
                
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Report a Bug"))
            {
                
            }

            ImGui::Separator();
            
            if(ImGui::MenuItem("Check for Updates"))
            {
                
            }

            if(ImGui::MenuItem("About"))
            {
                
            }

            ImGui::EndMenu();

        }
        
        ImGui::EndMainMenuBar();
    }
}
