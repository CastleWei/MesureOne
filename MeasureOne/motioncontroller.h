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
// 		int stepsToGo; //��ָ���Ĳ�����=0ʱһֱ��
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
// 	int dt = 500; //������ʱ�䣬΢��
// 	HANDLE hDevice = INVALID_HANDLE_VALUE;
// 	AxisInfo axes[AxisAll];
// 	uchar inBits[2], outBits[2]; //����/����ĵ�/��λ�ֽ�
// 	
// 	bool setOutBits(Axis axis);
// 	virtual void run();
// };
// 
// #endif // MOTIONCONTROLLER_H
