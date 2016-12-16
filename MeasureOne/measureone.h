#ifndef MEASUREONE_H
#define MEASUREONE_H

#include <QtWidgets/QMainWindow>
#include "ui_measureone.h"
#include "cameracontroller.h"
#include "imageprocess.h"

class MeasureOne : public QMainWindow
{
	Q_OBJECT

public:
	MeasureOne(QWidget *parent = 0);
	~MeasureOne();

private:
	Ui::MeasureOneClass ui;
	QActionGroup *grpSrcType;
	QLabel *statusFrameIntvl;
	QLabel *statusCalibration;
	QLabel *statusMotion;
	CameraController *cam = NULL;
	ImageProcess imgProc;
	
	int iTmrFps;

	void readPipelins(QComboBox *cmb);
	void savePipelins(QComboBox *cmb);

	virtual void timerEvent(QTimerEvent * event);

private slots:
	void OnTest();
	void OnCapture(bool checked);
	void OnPplBtnClicked(bool checked);
	void OnCmbIndexChanged(int index);
	void OnSrcTypeSelected(QAction *action);
};

#endif // MEASUREONE_H
