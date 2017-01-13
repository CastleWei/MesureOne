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

	int dt = 0; //ÿһ֡�ĺ�ʱ
	int camNum = 0; //��������
	bool isPlaying = true;
	bool willRest = true; //ÿ֮֡���Ƿ���Ϣ
	int msecRest = 15; //ÿ֮֡����Ϣ�ĺ�����

	//¼�����
	bool recording = false;
	bool willRecord = false;
	int nRcdFrm = 0; //��ǰ�����֡��
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

	//�ͽ������йص�һ�ɲ�����
	//bool jump = false; //�Ƿ��ý�������ת
	//float newpos = 0.0f; //��������������λ�ã�0~1
	//float curpos = 0.0f; //��������ǰλ��
	//QMutex camMutex; //�Ľ��ȵ�ʱ���õ�mutex
	SrcType srcType = fromCam;
	QString path; //��Ƶ����ͼƬ�ĵ�ַ

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
	void runPic(); //ͼ����ԴΪͼƬ
	void runVideoCam(); //ͼ����ԴΪ��Ƶ�������
};

#endif // CAMERACONTROLLER_H
