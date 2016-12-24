#include "motioncontroller.h"

MotionController::MotionController(QObject *parent)
: QThread(parent)
{
	//初始化各个轴的数据，如输入位
	axes[X].pulseBit = 0;
	axes[X].dirBit = 1;
	axes[X].offBit = 2;

	axes[Y].pulseBit = 3;
	axes[Y].dirBit = 4;
	axes[Y].offBit = 5;

	axes[Z].pulseBit = 8;
	axes[Z].dirBit = 9;
	axes[Z].offBit = 10;

}

MotionController::~MotionController()
{
	unConnect();
}

bool MotionController::setSpeed(int speed)
{
	bool ok = true; //返回是否成功设置到想要的值
	if (speed < 1){
		speed = 1;
		ok = false;
	}
	else if (speed > 100){
		speed = 100;
		ok = false;
	}

	//总微秒数，除以频率（以50Hz为单位）
	this->dt  = 1000 * 1000 / (speed * 50);

	return ok;
}

bool MotionController::go(Axis axis, Direction dir, int steps /*= 0*/)
{
	if (!isConnected()) return false;
	AxisInfo &a = axes[axis];

	a.isWorking = true;
	a.dir = dir;
	a.stepsToGo = steps;
	this->isWorking = true;
	this->start();

	return true;
}

bool MotionController::stop(Axis axis /*= AxisAll*/)
{
	if (axis == AxisAll){
		for (int i = 0; i < AxisAll; i++)
			axes[i].isWorking = false;
		this->isWorking = false;
		return !isWorking;
	}
	
	//停止某一个轴
	axes[axis].isWorking = false;
	//检查是否所有轴都停了
	bool willWork = false;
	for (int i = 0; i < AxisAll; i++){
		if (axes[i].isWorking)
			willWork = true;
	}
	this->isWorking = willWork;
	return !willWork;
}

void MotionController::uudelay(long usec)
{
	LARGE_INTEGER freq, t0, t1;
	QueryPerformanceCounter(&t0);
	t1 = t0;
	QueryPerformanceFrequency(&freq);
	double dfFreq = (double)freq.QuadPart / 1000 / 1000; //每微秒的频率
	long diff = (long)(dfFreq * usec); //要等待的tick数目

	while (t1.QuadPart - t0.QuadPart < diff)
		QueryPerformanceCounter(&t1);
}

inline void MotionController::setBit(quint16 &bits, int index, bool val)
{
	if (val)
		bits |= 1 << index;
	else
		bits &= ~(1 << index);
}

void MotionController::run()
{
	qDebug() << "motion run";
	while (isWorking){
		//待输出的两个字节outBits，pulseMask标记脉冲位
		quint16 outBits = 0, pulseMask = 0;

		for (int i = 0; i < AxisAll; i++){
			AxisInfo &a = axes[i];
			if (!a.isWorking) continue;

			setBit(pulseMask, a.pulseBit, true);
			setBit(outBits, a.dirBit, a.dir == Positive);
			setBit(outBits, a.offBit, a.isOff);

			a.pos += (a.dir == Positive) ? 1 : -1;

			//如果大于零，说明是按步数走的
			//走剩余的步数直到走完
			if (a.stepsToGo>0){
				a.stepsToGo--;
				if (a.stepsToGo == 0)
					stop(a.axisId);
			}
		}
		//正脉冲
		outBits |= pulseMask;
		MP441_DO(hDevice, 0, outBits);
		MP441_DO(hDevice, 1, outBits >> 8);
		uudelay(dt); //调节脉冲间隔

		//负脉冲
		outBits &= ~pulseMask;
		MP441_DO(hDevice, 0, outBits);
		MP441_DO(hDevice, 1, outBits >> 8);
		uudelay(dt); //调节脉冲间隔

	}
	qDebug() << "motion end";

}

