#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <string>

class QWidget;

std::string GetInputFromPrompt ( const std::string& title, const std::string& displayText, const std::string& defaultValue = std::string() );
std::string OpenGLMDialog ( QWidget *parent, const char *directory );

#endif