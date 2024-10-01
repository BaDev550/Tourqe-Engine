#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <imgui.h>

// Log levels
enum LogLevel {
    INFO,
    WARNING,
    ERROR,
};

// Custom stream buffer to capture std::cout output
class CoutRedirector : public std::streambuf {
public:
    CoutRedirector(std::vector<std::pair<std::string, LogLevel>>& log);
protected:
    int overflow(int c) override;

private:
    std::vector<std::pair<std::string, LogLevel>>& log; // Vector to store log messages with levels
    std::ostringstream buffer;      // Buffer to temporarily store the log message
};

// Function to render the log window
void RenderLogWindow(const std::vector<std::pair<std::string, LogLevel>>& log);

#endif // LOGGER_H
