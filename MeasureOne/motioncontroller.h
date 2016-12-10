// #ifndef MOTIONCONTROLLER_H
// #define MOTIONCONTROLLER_H
// 
// #include <QThread>
// #include <windows.h>
// 
// HANDLE MP441_OpenDevice();
// void MP441_Out(int HL, int bits);
// 
// enum Axis{
// 	X, Y, Z, AxisAll
// };
// 
// enum Direction{
// 	Positive, Negative
// };
// 
// class MotionController : public QThread
// {
// 	struct AxisInfo
// 	{
// 		Axis axisId;
// 		bool isWorking;
// 		Direction dir;
// 		int xpos;
// 		int ypos;
// 		int pulseBit;
// 		int dirBit;
// 		int offBit;
// 		int stepsToGo; //走指定的步数，=0时一直走
// 	};
// 
// 	Q_OBJECT
// 
// public:
// 	MotionController(QObject *parent);
// 	~MotionController();
// 
// 	bool connect();
// 	bool isConnected(){ return hDevice != INVALID_HANDLE_VALUE; }
// 	bool unConnect();
// 	bool go(Axis axis, Direction dir, int steps = 0);
// 	bool stop(Axis axis = AxisAll);
// 
// private:
// 	bool isWorking = false;
// 	int dt = 500; //脉冲间隔时间，微秒
// 	HANDLE hDevice = INVALID_HANDLE_VALUE;
// 	AxisInfo axes[AxisAll];
// 	uchar inBits[2], outBits[2]; //输入/输出的低/高位字节
// 	
// 	bool setOutBits(Axis axis);
// 	virtual void run();
// };
// 
// #endif // MOTIONCONTROLLER_H
