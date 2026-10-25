// HiddenInput.h - reads a password without showing it on the screen.
// If the input is not a real console (e.g. text piped in for testing), it
// simply reads the line normally.
#pragma once

#include <iostream>
#include <string>

std::string readHiddenLine(std::istream& in, std::ostream& out);
