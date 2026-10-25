#include "HiddenInput.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

static std::string readPlainLine(std::istream& in) {
    std::string line;
    std::getline(in, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

std::string readHiddenLine(std::istream& in, std::ostream& out) {
    if (&in != &std::cin) return readPlainLine(in);  // only hide on the real keyboard

#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode = 0;
    if (!GetConsoleMode(console, &oldMode)) return readPlainLine(in);  // not a console
    SetConsoleMode(console, oldMode & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
    std::string line = readPlainLine(in);
    SetConsoleMode(console, oldMode);
#else
    termios oldSettings{};
    if (tcgetattr(STDIN_FILENO, &oldSettings) != 0) return readPlainLine(in);  // not a terminal
    termios hidden = oldSettings;
    hidden.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
    std::string line = readPlainLine(in);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif
    out << "\n";  // the Enter key was not shown either, so move to the next line
    return line;
}
