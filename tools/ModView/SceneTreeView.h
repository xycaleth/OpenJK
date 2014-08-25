#ifndef SCENETREEVIEW_H
#define SCENETREEVIEW_H

#include <QtCore/QString>
#include "model.h"

class SceneTreeModel;

void SetupSceneTreeModel ( const QString& modelName, ModelContainer_t& container, SceneTreeModel& model );
void ClearSceneTreeModel ( SceneTreeModel& model );

#endif