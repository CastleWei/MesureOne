#include "vwdb.h"
#include <QTextEdit>
#include <QStatusBar>
#include <QDebug>
#include <QMessageBox>

QString beforeInit;
QTextEdit *out = nullptr;
QStatusBar *statout = nullptr; //×´Ì¬À¸

bool vwdb::initdb(void *_out, void *_statout /*= nullptr*/)
{
	if (!_out) return false;
	out = (QTextEdit*)_out;

	if (_statout != nullptr)
		statout = (QStatusBar *)_statout;

	if (!beforeInit.isEmpty())
		out->setText(beforeInit);

	return true;
}

bool vwdb::println(QString text)
{
	qDebug() << text;
	if (!out){
		beforeInit.append(text).append("\n");
		return false;
	}
	else{
		out->append(text);
		return true;
	}
}

bool vwdb::printstat(QString text, int timeout /*= 5000*/)
{
	bool ok1 = println(text);
	bool ok2 = statout != nullptr;
	if (ok2)
		statout->showMessage(text, timeout);
	return ok1 && ok2;
}

bool vwdb::showstat(QString text, int timeout /*= 5000*/)
{
	bool ok = statout != nullptr;
	if (ok)
		statout->showMessage(text, timeout);
	return ok;
}

bool vwdb::msgbox(QString text, QString title /*= ""*/)
{
	QMessageBox::information(nullptr, title, text);
	return true;
}
