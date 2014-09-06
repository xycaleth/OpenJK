#ifndef CHANGEFLOORDIALOG_H
#define CHANGEFLOORDIALOG_H

#include <QDialog>
#include "ui_ChangeFloor.h"

struct ModViewAppVars_t;

class ChangeFloorDialog : public QDialog
{
	Q_OBJECT

public:
	ChangeFloorDialog( ModViewAppVars_t& appVars, QWidget *parent = 0 );

private slots:
	void OnFloorHeightChanged( int y );
	void OnCancel();

private:
	Ui::ChangeFloorDlg ui;
	ModViewAppVars_t& appVars;

	float originalFloorZ;
};

#endif
