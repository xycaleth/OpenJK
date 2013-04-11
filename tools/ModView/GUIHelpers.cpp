#include "stdafx.h"
#include "GUIHelpers.h"

#include <QtWidgets/QInputDialog>

std::string GetInputFromPrompt ( const std::string& title, const std::string& displayText, const std::string& defaultValue )
{
    bool okay;
    QString input = QInputDialog::getText (0, QString::fromStdString (title), QString::fromStdString (displayText), QLineEdit::Normal, QString::fromStdString (defaultValue), &okay);
    if ( okay && !input.isEmpty() )
    {
        return input.toStdString();
    }

    return std::string();
}