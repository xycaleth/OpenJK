#include "ChangeFloorDialog.h"

#include "model.h"

ChangeFloorDialog::ChangeFloorDialog( ModViewAppVars_t& appVars, QWidget *parent )
	: QDialog(parent)
	, appVars(appVars)
	, originalFloorZ(appVars.fFloorZ)
{
	ui.setupUi( this );
}

void ChangeFloorDialog::OnFloorHeightChanged( int y )
{
	ui.heightTextbox->setText(QString::number(y));

	appVars.fFloorZ = static_cast<float>(y);
	ModelList_ForceRedraw();
}

void ChangeFloorDialog::OnCancel()
{
	appVars.fFloorZ = originalFloorZ;
	ModelList_ForceRedraw();

	reject();
}

