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
		bool isOff = false; //�Ƿ��ѻ�
		int pulseBit = 16; //��ʼֵ16��������Χ�����Զ��ضϣ��൱��û�����
		int dirBit = 16;
		int offBit = 16;
		int stepsToGo = 0; //��ָ���Ĳ�����=0ʱһֱ��
	};

	Q_OBJECT

public:
	MotionController(QObject *parent);
	~MotionController();

	bool isWorking = false;
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	AxisInfo axes[AxisAll];

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
	bool setSpeed(int speed); //���٣�1~100����50HzΪ��λ��50~5kHz
	bool go(Axis axis, Direction dir, int steps = 0);
	bool stop(Axis axis = AxisAll);
	int pos(Axis axis){ return axes[axis].pos; }

private:
	int dt = 2000; //��������ʱ�䣬΢��
	void uudelay(long usec); //�ӳ�usec��΢��
	inline void setBit(quint16 &bits, int index, bool val);
	virtual void run();
};

#endif // MOTIONCONTROLLER_H
