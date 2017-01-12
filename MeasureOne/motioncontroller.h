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
		bool isOff = false; //是否脱机
		int pulseBit = 16; //初始值16，超出范围，被自动截断，相当于没有输出
		int dirBit = 16;
		int offBit = 16;
		int stepsToGo = 0; //走指定的步数，=0时一直走
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
		return !isConnected(); //返回是否成功断开
	}
	bool setSpeed(int speed); //调速，1~100，以50Hz为单位，50~5kHz
	bool go(Axis axis, Direction dir, int steps = 0);
	bool stop(Axis axis = AxisAll);
	int pos(Axis axis){ return axes[axis].pos; }

private:
	int dt = 2000; //半脉冲间隔时间，微秒
	void uudelay(long usec); //延迟usec个微秒
	inline void setBit(quint16 &bits, int index, bool val);
	virtual void run();
};

#endif // MOTIONCONTROLLER_H
