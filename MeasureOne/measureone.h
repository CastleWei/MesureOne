#ifndef MEASUREONE_H
#define MEASUREONE_H

#include <QtWidgets/QMainWindow>
#include "ui_measureone.h"
#include "cameracontroller.h"

class MeasureOne : public QMainWindow
{
	Q_OBJECT

public:
	MeasureOne(QWidget *parent = 0);
	~MeasureOne();

private:
	Ui::MeasureOneClass ui;
	CameraController *cam = NULL;
	int iTmrFps;

	virtual void timerEvent(QTimerEvent * event);

private slots:
	void OnTest();
	void OnCapture(bool checked);
};

#endif // MEASUREONE_H
