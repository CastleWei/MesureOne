#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QDebug>
#include <QThread>
#include <QImage>
#include <QMutex>
#include <QFile>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>

#include "imageprocess.h"
#include "motioncontroller.h"

class CameraController : public QThread
{
	Q_OBJECT

public:
	enum SrcType{
		fromCam,
		fromVideo,
		fromPic
	};

public:
	CameraController(QObject *parent, ImageProcess &imgProc);
	~CameraController();

	int dt = 0; //每一帧的耗时
	int camNum = 0; //摄像机编号
	bool isPlaying = true;
	bool willRest = true; //每帧之间是否休息
	int msecRest = 15; //每帧之间休息的毫秒数

	//录像相关
	bool recording = false;
	bool willRecord = false;
	int nRcdFrm = 0; //当前保存的帧数
	QString recordPath;
	cv::VideoWriter vwrt;
	QFile motionStepsFile;
	MotionController *motion = nullptr;
	void startRecorder(){
		vwrt.open(recordPath.toStdString(),
			CV_FOURCC('M', 'P', '4', '2'), 25, imgObj.src.size());
		motionStepsFile.setFileName(recordPath + ".motionsteps.txt");
		motionStepsFile.open(QIODevice::WriteOnly | QIODevice::Text);
		if (vwrt.isOpened() && motionStepsFile.isOpen() && motion != nullptr) {
			nRcdFrm = 0;
			recording = true;
			//vwdb::printstat("record begin");
			qDebug() << "record begin";
		}
		else{
			willRecord = false;
			recording = false;
			//vwdb::printstat("record fail");
			qDebug() << "record begin";
		}
	}
	void stopRecorder(){
		vwrt.release();
		motionStepsFile.close();
		recordPath = "";
		recording = false;
		//vwdb::printstat("record end");
		qDebug() << "record begin";
	}

	//和进度条有关的一律不能用
	//bool jump = false; //是否用进度条跳转
	//float newpos = 0.0f; //进度条跳到的新位置，0~1
	//float curpos = 0.0f; //进度条当前位置
	//QMutex camMutex; //改进度的时候用的mutex
	SrcType srcType = fromCam;
	QString path; //视频或者图片的地址

	ImageObject imgObj;
	ImageProcess &imgProc;

	void start();
	void end();

signals:
	void imageCollected();
	void finishCollecting();

private:
	bool isOn = false;
	QElapsedTimer elpsTmr;
	
	void run();
	void runPic(); //图像来源为图片
	void runVideoCam(); //图像来源为视频或摄像机
};

#endif // CAMERACONTROLLER_H
