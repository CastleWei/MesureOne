#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <QThread>
#include <QDebug>
#include <windows.h>
#pragma comment(lib,"MP441.lib")
#include "MP441.h"

enum Axis{
	X, Y, Z, AxisAll
};

enum Direction{
	Positive, Negative
};

class MotionController : public QThread
{
	struct AxisInfo
	{
		Axis axisId = X;
		bool isWorking = false;
		Direction dir = Positive;
		int pos = 0; //��ǰλ�ã���������ǣ�
		int pulseBit = 0;
		int dirBit = 0;
		int offBit = 0;
		int stepsToGo = 0; //��ָ���Ĳ�����=0ʱһֱ��
	};

	Q_OBJECT

public:
	MotionController(QObject *parent);
	~MotionController();

	bool isWorking = false;
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	AxisInfo axes[AxisAll];
	int dt = 2000; //��������ʱ�䣬΢��

	bool connect(){ hDevice = MP441_OpenDevice(0); return isConnected(); }
	bool isConnected(){ return hDevice != INVALID_HANDLE_VALUE; }
	bool unConnect(){
		if (isConnected()){
			isWorking = false;
			wait();
			MP441_CloseDevice(hDevice);
			hDevice = INVALID_HANDLE_VALUE;
		}
		return !isConnected(); //�����Ƿ�ɹ��Ͽ�
	}
	bool go(Axis axis, Direction dir, int steps = 0);
	bool stop(Axis axis = AxisAll);

private:

	//����/����ĵ�/��λ�ֽ�
	uchar inLB = 0, inHB = 0, outLB = 0, outHB = 0;
	void uudelay(long usec); //�ӳ�usec��΢��
	bool setOutBits(AxisInfo &a);
	virtual void run();
};

#endif // MOTIONCONTROLLER_H
