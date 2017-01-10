//自定义调试类

#ifndef VWDB_H
#define VWDB_H

#include <QString>

namespace vwdb{
	bool initdb(void *_out, void *_statout = nullptr);
	bool println(QString text);
	bool printstat(QString text, int timeout = 5000); //print并状态栏显示
	bool showstat(QString text, int timeout = 5000); //仅状态栏显示
	bool msgbox(QString text, QString title = ""); //弹出信息框
}

#endif // VWDB_H

