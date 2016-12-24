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

bool MotionController::setOutBits(AxisInfo &a)
{
	//与某一位为一的数异或，该位取反；
	//&7即取除8的余数
	if (a.pulseBit < 8)
		outLB ^= 1 << a.pulseBit;
	else
		outHB ^= 1 << (a.pulseBit & 7);

	uchar &hl = (a.dirBit < 8) ? outLB : outHB;
	int bit = a.dirBit & 7;
	if (a.dir == Positive)
		hl |= 1 << bit;
	else
		hl &= ~(1 << bit);

	//if (a.offBit < 8)
	//	outLB &= 1 << a.offBit;
	//else
	//	outHB &= 1 << (a.offBit & 7);

	return true;
}

void MotionController::run()
{
	qDebug() << "motion run";
	while (isWorking){
		for (int i = 0; i < AxisAll; i++){
			AxisInfo &a = axes[i];
			if (!a.isWorking) continue;

			setOutBits(a); //把输出字节该取反的取反后，才一起输出

			a.pos += (a.dir == Positive) ? 1 : -1;

			//如果大于零，说明是按步数走的
			//走剩余的步数直到走完
			if (a.stepsToGo>0){
				a.stepsToGo--;
				if (a.stepsToGo == 0)
					stop(a.axisId);
			}
		}
		MP441_DO(hDevice, 0, outLB);
		MP441_DO(hDevice, 1, outHB);
		//调节脉冲间隔
		uudelay(2);
	}
	qDebug() << "motion end";

}

