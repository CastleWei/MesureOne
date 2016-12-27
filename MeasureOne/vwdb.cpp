#include "vwdb.h"
#include <QTextEdit>

QString beforeInit;
QTextEdit *out = nullptr;


bool vwdb::initdb(void *_out)
{
	if (!_out) return false;
	out = (QTextEdit*)_out;

	if (!beforeInit.isEmpty())
		out->append(beforeInit);

	return true;
}

bool vwdb::println(QString text)
{
	if (!out){
		beforeInit.append(text).append("\n");
		return false;
	}
	else{
		out->append(text);
		return true;
	}
}
