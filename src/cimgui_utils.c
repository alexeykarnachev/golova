#include "cimgui_utils.h"

#include "raylib.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>

ImGuiWindowFlags GHOST_WINDOW_FLAGS
    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus
      | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoInputs;

void load_imgui(void) {
    igCreateContext(NULL);
    GLFWwindow* window = (GLFWwindow*)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    igStyleColorsDark(NULL);
}

void begin_imgui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

void end_imgui(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void ig_fix_window_top_left(void) {
    ImVec2 position = {0.0, 0.0};
    igSetNextWindowPos(position, ImGuiCond_Always, (ImVec2){0.0, 0.0});
    igSetNextWindowSize((ImVec2){0.0, 0.0}, ImGuiCond_Always);
}

void ig_fix_window_bot_left(void) {
    ImGuiIO* io = igGetIO();
    ImVec2 position = {0, io->DisplaySize.y};
    ImVec2 pivot = {0, 1};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize((ImVec2){0.0, 0.0}, ImGuiCond_Always);
}

bool ig_collapsing_header(const char* name, bool is_opened) {
    int flags = is_opened ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    return igCollapsingHeader_TreeNodeFlags(name, flags);
}
