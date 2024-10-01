#include "Logger.h"


CoutRedirector::CoutRedirector(std::vector<std::pair<std::string, LogLevel>>& log) : log(log) {}

int CoutRedirector::overflow(int c) {
    if (c == '\n') {
        std::string message = buffer.str();
        LogLevel level = INFO;

        if (message.find("[WARNING]") != std::string::npos) {
            level = WARNING;
            message.erase(message.find("[WARNING]"), 9);
        }
        else if (message.find("[ERROR]") != std::string::npos) {
            level = ERROR;
            message.erase(message.find("[ERROR]"), 7);
        }

        log.push_back({ message, level });
        buffer.str("");
        buffer.clear();
    }
    else {
        buffer << static_cast<char>(c);
    }
    return c;
}

// Function to render the log window
void RenderLogWindow(const std::vector<std::pair<std::string, LogLevel>>& log) {

    for (const auto& entry : log) {
        switch (entry.second) {
        case WARNING:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("[WARNING] %s", entry.first.c_str());
            ImGui::PopStyleColor();
            break;
        case ERROR:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("[ERROR] %s", entry.first.c_str());
            ImGui::PopStyleColor();
            break;
        case INFO:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.2f, 1.0f));
            ImGui::Text("[INFO] %s", entry.first.c_str());
            ImGui::PopStyleColor();
            break;
        }
    }

}
