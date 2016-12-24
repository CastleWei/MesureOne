#ifndef CALIBR_H
#define CALIBR_H

#include "motioncontroller.h"
#include "imageprocess.h"
#include <QDebug>
#include <QObject>
#include <QVector>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

bool textoutput(QList<QPointF>, const char *str);

class Calibr : public QObject
{
	Q_OBJECT

public:
	Calibr(MotionController &motion, ImageProcess &imgProc, ImageObject &imgObj);
	~Calibr();
	virtual void timerEvent(QTimerEvent *);

	QList<QPointF> samples; //采样点列，p.x为走的毫米数，p.y为对应光刀偏移量
	QVector<float> lut; //标定结果，look-up-table

	void takesamples(); //采样
	float getTwoLines(ImageObject &imgObj, float &off); //找出基线和光刀偏移量
	//void calc(); //计算插值得出lut的值

	void onA();
	void onB();
	void onCaliGo();
	void onCaliStop();
	void onCaliCalc(float realAB, float z0, float ze, int width);

private:
	MotionController &motion;
	ImageProcess &imgProc;
	ImageObject &imgObj;

	int a, b, abmm; //步数标定的横向起点，终点，实际长度
	float mmperstep; //每步几毫米
	float baseline; //基线偏移量
	int beginpos; //偏移量标定时 开始的步数
	int maxoffs; //最大偏移量，与图像宽度有关

	int sampleIntvl = 100; //采样延迟
	int tmr; //定时采样的timer
};

#endif // CALIBR_H
