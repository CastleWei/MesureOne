//open MP441
extern "C" HANDLE __declspec(dllexport)  __stdcall MP441_OpenDevice(long dev_num);
//close Device
extern "C" long __declspec(dllexport)  __stdcall MP441_CloseDevice(HANDLE hDevice1);
//********************************************
//get board info
//model or type in *bStr
extern "C" long __declspec(dllexport)  __stdcall MP441_Info(HANDLE hDevice,char *modle);
//read DI
extern "C" long __declspec(dllexport)  __stdcall MP441_DI(HANDLE hDevice,long ionum);
//write do
extern "C" long __declspec(dllexport)  __stdcall MP441_DO(HANDLE hDevice,long ionum,long iodata);
