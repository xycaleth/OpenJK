#include "stdafx.h"
#include "GUIHelpers.h"

#include <QtWidgets/QInputDialog>
#include <QFileDialog>

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

std::string OpenGLMDialog ( QWidget *parent, const char *directory )
{
	QFileDialog openDialog (parent);
    openDialog.setDirectory (QString::fromLatin1 (directory));
    openDialog.setFileMode (QFileDialog::ExistingFile);
    openDialog.setNameFilter (QObject::tr ("Model files (*.glm)"));
    openDialog.setAcceptMode (QFileDialog::AcceptOpen);

    if ( openDialog.exec() )
    {
        QStringList modelName = openDialog.selectedFiles();
        if ( !modelName.isEmpty() )
        {
			return modelName[0].toStdString();
        }
    }

	return "";
}