#ifndef MEASUREONE_H
#define MEASUREONE_H

#include <QtWidgets/QMainWindow>
#include "ui_measureone.h"
#include "cameracontroller.h"
#include "motioncontroller.h"
#include "imageprocess.h"
#include "calibr.h"

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
	CameraController *cam = nullptr;
	MotionController *motion = nullptr;
	Calibr *cali = nullptr;
	ImageProcess imgProc;
	int tmr; //每隔一秒显示运算耗时

	void readPipelins(QComboBox *cmb);
	void savePipelins(QComboBox *cmb);
	QImage getQimg(ImageObject &imgObj);
	virtual void timerEvent(QTimerEvent *event);

private slots:
	void showImg();
	void finishCollecting();

	void OnTest();
	void OnTry();
	void OnCapture(bool checked);
	void OnPlayPause(bool checked);
	void OnFastPlay(bool checked);
	void OnPplBtnClicked(bool checked);
	void OnCmbIndexChanged(int index);
	void OnSrcTypeSelected(QAction *action);
	void OnMotionConnect(bool checked);
	void OnMotionUp(bool checked);
	void OnMotionDown(bool checked);
	void OnMotionForw(bool checked);
	void OnMotionBack(bool checked);
	void OnMotionLeft(bool checked);
	void OnMotionRight(bool checked);
	void OnMotionStop(bool checked);
	void OnCaliA();
	void OnCaliB();
	void OnCaliGo();
	void OnCaliStop();
	void OnCaliCalc();
	
	//void OnProgressChanged(int value);
};

#endif // MEASUREONE_H
