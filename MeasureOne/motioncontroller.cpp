#include "motioncontroller.h"

MotionController::MotionController(QObject *parent)
: QThread(parent)
{
	//��ʼ������������ݣ�������λ
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
	bool ok = true; //�����Ƿ�ɹ����õ���Ҫ��ֵ
	if (speed < 1){
		speed = 1;
		ok = false;
	}
	else if (speed > 100){
		speed = 100;
		ok = false;
	}

	//��΢����������Ƶ�ʣ���50HzΪ��λ��
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
	
	//ֹͣĳһ����
	axes[axis].isWorking = false;
	//����Ƿ������ᶼͣ��
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
	double dfFreq = (double)freq.QuadPart / 1000 / 1000; //ÿ΢���Ƶ��
	long diff = (long)(dfFreq * usec); //Ҫ�ȴ���tick��Ŀ

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
		//������������ֽ�outBits��pulseMask�������λ
		quint16 outBits = 0, pulseMask = 0;

		for (int i = 0; i < AxisAll; i++){
			AxisInfo &a = axes[i];
			if (!a.isWorking) continue;

			setBit(pulseMask, a.pulseBit, true);
			setBit(outBits, a.dirBit, a.dir == Positive);
			setBit(outBits, a.offBit, a.isOff);

			a.pos += (a.dir == Positive) ? 1 : -1;

			//��������㣬˵���ǰ������ߵ�
			//��ʣ��Ĳ���ֱ������
			if (a.stepsToGo>0){
				a.stepsToGo--;
				if (a.stepsToGo == 0)
					stop(a.axisId);
			}
		}
		//������
		outBits |= pulseMask;
		MP441_DO(hDevice, 0, outBits);
		MP441_DO(hDevice, 1, outBits >> 8);
		uudelay(dt); //����������

		//������
		outBits &= ~pulseMask;
		MP441_DO(hDevice, 0, outBits);
		MP441_DO(hDevice, 1, outBits >> 8);
		uudelay(dt); //����������

	}
	qDebug() << "motion end";

}

