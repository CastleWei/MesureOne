#include "measureone.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MeasureOne w;
	w.show();
	return a.exec();
}
