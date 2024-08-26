#include "imtui/imtui-impl-ncurses.h"
#include "imtui/imtui.h"

int main() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImTui_ImplText_Init();
    auto g_screen = ImTui_ImplNcurses_Init(true);
    while (1) {
        
    }
    return 0;
}
