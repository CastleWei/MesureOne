#include "measureone.h"
#include <QFileDialog>
#include <QTimerEvent>

MeasureOne::MeasureOne(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	cam = new CameraController(nullptr);
	connect(ui.actTest, SIGNAL(triggered()), this, SLOT(OnTest()));
	connect(ui.actCapture, SIGNAL(triggered(bool)), this, SLOT(OnCapture(bool)));

}

MeasureOne::~MeasureOne()
{
	//cam->end();
}

void MeasureOne::timerEvent(QTimerEvent * event)
{
	if (event->timerId() == iTmrFps){
		int dt = (int)cam->dt;
		ui.statusBar->showMessage(QString::number(dt)+QString("ms"));
	}
}

void MeasureOne::OnTest()
{
	//QString file = QFileDialog::getOpenFileName(this, "test");
	//QImage img(file);
	//ui.statusBar->showMessage("ready");

}

void MeasureOne::OnCapture(bool checked)
{
	if (checked){
		cam->start();
		ui.widgShow->startRefreshing(cam);
		iTmrFps = startTimer(1000); //刷新计算时间
	}
	else
	{
		cam->end();
		ui.widgShow->stopRefreshing();
		killTimer(iTmrFps);
	}
}
