#include "TextEditor.h"
#include <imgui.h>
#include <stdexcept>
#include <iostream>

static void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static void Run(bool reverse) {
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800, 800);
    io.DeltaTime = 1.0f / 60;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    {
        TextEditor editor;
        TextEditor::AutoCompleteConfig config;
        config.triggerOnTyping = false;
        config.triggerDelay = std::chrono::milliseconds(0);
        int requests = 0;
        config.callback = [&](TextEditor::AutoCompleteState& state) {
            ++requests;
            state.suggestionsPromise = true;
        };
        editor.SetAutoCompleteConfig(&config);
        auto frame = [&](int focus = -1, int hidden = -1) {
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(750, 750));
            ImGui::Begin("Shared document");
            for (int index = 0; index != 2; ++index) {
                int view = reverse ? 1 - index : index;
                if (view == hidden) continue;
                ImGui::PushID(view);
                if (view == focus) editor.SetFocus();
                editor.Render("source", ImVec2(650, 260));
                ImGui::PopID();
            }
            ImGui::End();
            ImGui::Render();
        };
        auto trigger = [&] {
            io.AddKeyEvent(ImGuiMod_Ctrl, true);
            io.AddKeyEvent(ImGuiKey_Space, true);
            frame();
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
            io.AddKeyEvent(ImGuiKey_Space, false);
            frame(); frame();
        };
        frame(); frame();
        for (int focused = 0; focused != 2; ++focused) {
            editor.SetText("pri");
            editor.SetCursor(TextEditor::DocPos(0, 3));
            frame(focused); frame(); frame();
            int before = requests;
            trigger();
            Require(requests == before + 1, "Completion request was lost or duplicated");
            Require(editor.IsAutoCompleteActive(), "Other view canceled completion");
            editor.SetAutoCompleteSuggestions({"print"});
            frame(); frame();
            io.AddKeyEvent(ImGuiKey_Enter, true);
            frame();
            io.AddKeyEvent(ImGuiKey_Enter, false);
            frame(); frame();
            if (editor.GetText() != "print")
                throw std::runtime_error("Completion was not accepted once: [" + editor.GetText() + "]");
            editor.Undo();
            Require(editor.GetText() == "pri", "Completion lost shared undo history");
        }
        editor.SetCursor(TextEditor::DocPos(0, 3));
        frame(0); frame(); frame();
        trigger();
        Require(editor.IsAutoCompleteActive(), "Owner did not open completion");
        frame(1, 0); frame(-1, 0);
        Require(!editor.IsAutoCompleteActive(), "Hidden owner left a stale completion in another view");
    }
    ImGui::DestroyContext();
}

int main() {
    try { Run(false); Run(true); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    return 0;
}
