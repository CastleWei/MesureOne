//�Զ��������

#ifndef VWDB_H
#define VWDB_H

#include <QString>

namespace vwdb{
	bool initdb(void *_out, void *_statout = nullptr);
	bool println(QString text);
	bool printstat(QString text, int timeout = 5000); //print��״̬����ʾ
	bool showstat(QString text, int timeout = 5000); //��״̬����ʾ
	bool msgbox(QString text, QString title = ""); //������Ϣ��
}

#endif // VWDB_H

