#include "measureone.h"
#include "common.h"
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>
#include <QDebug>

MeasureOne::MeasureOne(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	cam = new CameraController(nullptr);

	//图像来源 单选菜单组
	grpSrcType = new QActionGroup(this);
	grpSrcType->addAction(ui.actFromPic);
	grpSrcType->addAction(ui.actFromVideo);
	grpSrcType->addAction(ui.actFromCam);
	ui.actFromCam->setChecked(true);

	//初始化状态栏
	statusMotion = new QLabel(this);
	// 	statusMotion->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusMotion->setMinimumWidth(150);
	statusMotion->setIndent(4);
	statusMotion->setText(CN("电机未连接"));
	statusBar()->addPermanentWidget(statusMotion);

	statusCalibration = new QLabel(this);
	statusCalibration->setMinimumWidth(150);
	statusCalibration->setIndent(4);
	statusCalibration->setText(CN("未标定"));
	statusBar()->addPermanentWidget(statusCalibration);

	statusFrameIntvl = new QLabel(this);
	statusFrameIntvl->setMinimumWidth(150);
	statusFrameIntvl->setIndent(4);
	statusFrameIntvl->setText(CN("--ms"));
	statusBar()->addPermanentWidget(statusFrameIntvl);

	//读取设置文件
	readPipelins(ui.cmbPipeline);

	connect(ui.actTest, SIGNAL(triggered()), this, SLOT(OnTest()));
	connect(ui.actCapture, SIGNAL(triggered(bool)), this, SLOT(OnCapture(bool)));
	connect(ui.btnPipeline, SIGNAL(clicked(bool)), this, SLOT(OnPplBtnClicked(bool)));
	connect(grpSrcType, SIGNAL(triggered(QAction*)), this, SLOT(OnSrcTypeSelected(QAction*)));
	connect(ui.cmbPipeline, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCmbIndexChanged(int)));
}

MeasureOne::~MeasureOne()
{
	//cam->end();
}

//读取配置文件
void MeasureOne::readPipelins(QComboBox *cmb)
{
	cmb->clear();
	QFile file("pipelines.txt");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Can't read the file!" ;
		cmb->addItem("<default>", CN("极大值法")); //无文件时的默认方案
	}
	while (!file.atEnd()) {
		QString line(file.readLine());
		line = line.trimmed();
		if (line.startsWith("<") && line.endsWith(">")){
			//item头形如： <sample>
			//读取item头中的名字
			QString pplname = line.mid(1, line.length() - 2);
			//读取一个pipeline方案中的每一行
			QString cmds;
			while (1){
				QString cmd(file.readLine());
				if (cmd.trimmed() == "</>" || file.atEnd()) break; //pipeline结尾标识
				cmds.append(cmd);
			}
			//存入ComboBox中
			cmb->addItem(pplname, cmds);
		}
	}
}

void MeasureOne::savePipelins(QComboBox *cmb)
{
	QFile file("pipelines.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't write the file!";
		QMessageBox::information(this, CN("错误"), CN("保存文件失败"));
	}
	QTextStream tout(&file);
	for (int i = 0; i < cmb->count(); i++){
		QString pplname = cmb->itemText(i);
		QString cmds = cmb->itemData(i).toString();
		//tout << QString("<%1>").arg(pplname) << endl;
		tout << "<" << pplname << ">" << endl;
		tout << cmds.trimmed() << endl;
		tout << "</>" << endl << endl;
	}
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
	//if (checked){
	//	cam->start();
	//	ui.widgShow->startRefreshing(cam);
	//	iTmrFps = startTimer(1000); //刷新计算时间
	//}
	//else
	//{
	//	cam->end();
	//	ui.widgShow->stopRefreshing();
	//	killTimer(iTmrFps);
	//}
}

//算法流水线确定保存
void MeasureOne::OnPplBtnClicked(bool checked)
{
	QComboBox *cmb = ui.cmbPipeline;
	if (checked){
		//进入编辑状态
		int index = cmb->currentIndex();
		QString curText = cmb->currentText();
		QString itemText = cmb->itemText(index);

		//如果键入了新的名字，新建一个pipeline方案
		if (curText != itemText){
			//寻找已有的名字
			for (index = 0; index < cmb->count(); index++){
				if (curText == cmb->itemText(index)) break;
			}
			//找完如果没有
			if (index == cmb->count()){
				cmb->addItem(curText);
			}
		}

		ui.txtPipeline->setPlainText(cmb->itemData(index).toString());
		ui.txtPipeline->setReadOnly(false);
	}
	else{
		//解析、保存当前指令
		QString code = ui.txtPipeline->toPlainText();
		if (code == ""){
			QMessageBox::information(this, "", CN("即将删除本条目"));
			cmb->removeItem(cmb->currentIndex());
		}
		else{
			bool ok = imgProc.parse(code);
			if (!ok){
				QMessageBox::information(this,
					CN("解析失败"), CN("解释失败！\n请修改指令！"));
				ui.btnPipeline->setChecked(true);
				return;
			}
			//如果成功
			cmb->setItemData(cmb->currentIndex(), code);
		}
		ui.txtPipeline->setReadOnly(true);
		ui.txtPipeline->setPlainText(imgProc.toString(true));
		savePipelins(cmb);
	}
}

void MeasureOne::OnCmbIndexChanged(int index)
{
	QComboBox *cmb = ui.cmbPipeline;
	QString itemData = cmb->itemData(index).toString();

	if (ui.btnPipeline->isChecked()){
		ui.txtPipeline->setPlainText(itemData);
	}
	else{
		imgProc.parse(itemData);
		ui.txtPipeline->setPlainText(imgProc.toString(false));
	}
}

void MeasureOne::OnSrcTypeSelected(QAction *action)
{
	if (action == ui.actFromPic){
		QMessageBox::information(this, "", "from pic");
	}
	else if (action == ui.actFromVideo){
		QMessageBox::information(this, "", "from video");

	}
	else if (action == ui.actFromCam){
		QMessageBox::information(this, "", "from cam");

	}
}
