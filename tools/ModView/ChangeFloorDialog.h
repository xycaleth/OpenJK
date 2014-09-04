#ifndef CHANGEFLOORDIALOG_H
#define CHANGEFLOORDIALOG_H

#include <QDialog>
#include "ui_ChangeFloor.h"

class ChangeFloorDialog : public QDialog
{
	Q_OBJECT

public:
	ChangeFloorDialog( QWidget *parent = 0 );

private:
	Ui::ChangeFloorDlg ui;
};

#endif
