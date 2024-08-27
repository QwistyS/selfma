#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"

#include <string>
#include <vector>

struct Project {
    uint32_t id;
    std::string name;
    std::string decs;
};

// Simple UI state struct
struct UI {
    std::vector<Project> leftPanelProjects;
    int selectedItem = -1;
    std::vector<std::string> messages;
    std::string inputBuffer;
    bool showHelpWindow = false;
    int leftPanelWidth = 20;
};

// Global variables
UI g_ui;
bool g_isRunning = true;
ImTui::TScreen* g_screen = nullptr;

// Function to render the left panel
void renderLeftPanel() {
    ImGui::BeginChild("LeftPanel", ImVec2(g_ui.leftPanelWidth, 0), true);
    
    for (int i = 0; i < g_ui.leftPanelProjects.size(); i++) {
        if (ImGui::Selectable(g_ui.leftPanelProjects[i].name.c_str(), g_ui.selectedItem == i)) {
            g_ui.selectedItem = i;
        }
    }
    
    ImGui::EndChild();
}

// Function to render the main content area
void renderMainContent() {
    ImGui::BeginChild("MainContent", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);
    
    if (g_ui.selectedItem >= 0 && g_ui.selectedItem < g_ui.leftPanelProjects.size()) {
        ImGui::Text("Selected: %s", g_ui.leftPanelProjects[g_ui.selectedItem].name.c_str());
    }
    
    for (const auto& message : g_ui.messages) {
        ImGui::TextWrapped("%s", message.c_str());
    }
    
    ImGui::EndChild();
}

// Function to render a frame
bool render_frame() {
    ImTui_ImplNcurses_NewFrame();
    ImTui_ImplText_NewFrame();
    ImGui::NewFrame();

    // Set up the main window
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(g_screen->nx, g_screen->ny), ImGuiCond_Always);
    ImGui::Begin("Main Window", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
            if (ImGui::MenuItem("Help")) {
                g_ui.showHelpWindow = true;
            }
            if (ImGui::MenuItem("Quit")) {
                g_isRunning = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Render left panel
    renderLeftPanel();

    // Render main content area
    ImGui::SameLine();
    renderMainContent();

    static char input[1024];
    // Input box
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##input", input, sizeof(input), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (!g_ui.inputBuffer.empty()) {
            g_ui.messages.push_back(g_ui.inputBuffer);
            g_ui.inputBuffer.clear();
        }
    }
    ImGui::PopItemWidth();

    ImGui::End();

    // Help window
    if (g_ui.showHelpWindow) {
        ImGui::SetNextWindowSize(ImVec2(g_screen->nx / 2, g_screen->ny / 2), ImGuiCond_FirstUseEver);
        ImGui::Begin("Help", &g_ui.showHelpWindow);
        ImGui::Text("This is a simple TUI application with a left panel.");
        ImGui::Text("Select items in the left panel to display them.");
        ImGui::Text("Use the input box to enter messages.");
        ImGui::Text("Press Esc to quit.");
        ImGui::End();
    }

    ImGui::Render();
    ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), g_screen);
    ImTui_ImplNcurses_DrawScreen();

    return true;
}

int main() {
    // Initialize ImGui and ImTui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Initialize screen
    g_screen = ImTui_ImplNcurses_Init(true);
    ImTui_ImplText_Init();

    // Add some sample items to the left panel
    // g_ui.leftPanelItems = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};

    // Main loop
    while (g_isRunning) {
        if (!render_frame()) break;

        // Check for Esc key to quit
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
            g_isRunning = false;
        }
    }

    // Cleanup
    ImTui_ImplText_Shutdown();
    ImTui_ImplNcurses_Shutdown();

    return 0;
}
