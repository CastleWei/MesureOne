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
		int pos = 0; //当前位置（按步数标记）
		int pulseBit = 0;
		int dirBit = 0;
		int offBit = 0;
		int stepsToGo = 0; //走指定的步数，=0时一直走
	};

	Q_OBJECT

public:
	MotionController(QObject *parent);
	~MotionController();

	bool isWorking = false;
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	AxisInfo axes[AxisAll];
	int dt = 2000; //半脉冲间隔时间，微秒

	bool connect(){ hDevice = MP441_OpenDevice(0); return isConnected(); }
	bool isConnected(){ return hDevice != INVALID_HANDLE_VALUE; }
	bool unConnect(){
		if (isConnected()){
			isWorking = false;
			wait();
			MP441_CloseDevice(hDevice);
			hDevice = INVALID_HANDLE_VALUE;
		}
		return !isConnected(); //返回是否成功断开
	}
	bool go(Axis axis, Direction dir, int steps = 0);
	bool stop(Axis axis = AxisAll);

private:

	//输入/输出的低/高位字节
	uchar inLB = 0, inHB = 0, outLB = 0, outHB = 0;
	void uudelay(long usec); //延迟usec个微秒
	bool setOutBits(AxisInfo &a);
	virtual void run();
};

#endif // MOTIONCONTROLLER_H
