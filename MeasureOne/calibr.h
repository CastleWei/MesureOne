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

	QList<QPointF> samples; //�������У�p.xΪ�ߵĺ�������p.yΪ��Ӧ�⵶ƫ����
	QVector<float> lut; //�궨�����look-up-table

	void takesamples(); //����
	float getTwoLines(ImageObject &imgObj, float &off); //�ҳ����ߺ͹⵶ƫ����
	//void calc(); //�����ֵ�ó�lut��ֵ

	void onA();
	void onB();
	void onCaliGo();
	void onCaliStop();
	void onCaliCalc(float realAB, float z0, float ze, int width);

private:
	MotionController &motion;
	ImageProcess &imgProc;
	ImageObject &imgObj;

	int a, b, abmm; //�����궨�ĺ�����㣬�յ㣬ʵ�ʳ���
	float mmperstep; //ÿ��������
	float baseline; //����ƫ����
	int beginpos; //ƫ�����궨ʱ ��ʼ�Ĳ���
	int maxoffs; //���ƫ��������ͼ�����й�

	int sampleIntvl = 100; //�����ӳ�
	int tmr; //��ʱ������timer
};

#endif // CALIBR_H
