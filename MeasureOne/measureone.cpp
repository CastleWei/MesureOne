#include "measureone.h"
#include "common.h"
#include "vwdb.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>
#include <QTimerEvent>
#include <QMessageBox>
#include <QDebug>

MeasureOne::MeasureOne(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	cam = new CameraController(this, imgProc);
	motion = new MotionController(this);
	cali = new Calibr(*motion, imgProc, cam->imgObj);
	
	ui.lblMeasure->init(&cam->imgObj, &calidata);

	//ͼ����Դ ��ѡ�˵���
	grpSrcType = new QActionGroup(this);
	grpSrcType->addAction(ui.actFromPic);
	grpSrcType->addAction(ui.actFromVideo);
	grpSrcType->addAction(ui.actFromCam);
	ui.actFromCam->setChecked(true);

	//��ʼ��״̬��
	statusMotion = new QLabel(this);
	//statusMotion->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusMotion->setMinimumWidth(150);
	statusMotion->setIndent(4);
	statusMotion->setText(CN("���δ����"));
	statusBar()->addPermanentWidget(statusMotion);

	statusCalibration = new QLabel(this);
	statusCalibration->setMinimumWidth(150);
	statusCalibration->setIndent(4);
	statusCalibration->setText(CN("δ�궨"));
	statusBar()->addPermanentWidget(statusCalibration);

	statusFrameIntvl = new QLabel(this);
	statusFrameIntvl->setMinimumWidth(150);
	statusFrameIntvl->setIndent(4);
	statusFrameIntvl->setText(CN("--ms"));
	statusBar()->addPermanentWidget(statusFrameIntvl);

	//��ȡ�����ļ�
	readPipelins(ui.cmbPipeline);
	imgProc.parse(ui.cmbPipeline->itemData(ui.cmbPipeline->currentIndex()).toString());
	ui.txtPipeline->setPlainText(imgProc.toString(false));

	connect(cam, SIGNAL(imageCollected()), this, SLOT(showImg()));
	connect(cam, SIGNAL(imageCollected()), ui.lblMeasure, SLOT(update()));
	connect(cam, SIGNAL(finishCollecting()), this, SLOT(finishCollecting()));

	connect(ui.btnMotionUp, SIGNAL(clicked(bool)), this, SLOT(OnMotionUp(bool)));
	connect(ui.btnMotionDown, SIGNAL(clicked(bool)), this, SLOT(OnMotionDown(bool)));
	connect(ui.btnMotionForw, SIGNAL(clicked(bool)), this, SLOT(OnMotionForw(bool)));
	connect(ui.btnMotionBack, SIGNAL(clicked(bool)), this, SLOT(OnMotionBack(bool)));
	connect(ui.btnMotionLeft, SIGNAL(clicked(bool)), this, SLOT(OnMotionLeft(bool)));
	connect(ui.btnMotionRight, SIGNAL(clicked(bool)), this, SLOT(OnMotionRight(bool)));
	connect(ui.btnMotionStop, SIGNAL(clicked(bool)), this, SLOT(OnMotionStop(bool)));
	
	connect(ui.btnCaliA, SIGNAL(clicked()), this, SLOT(OnCaliA()));
	connect(ui.btnCaliB, SIGNAL(clicked()), this, SLOT(OnCaliB()));
	connect(ui.btnCaliGo, SIGNAL(clicked()), this, SLOT(OnCaliGo()));
	connect(ui.btnCaliStop, SIGNAL(clicked()), this, SLOT(OnCaliStop()));
	connect(ui.btnCaliCalc, SIGNAL(clicked()), this, SLOT(OnCaliCalc()));
	
	connect(ui.btnPlayPause, SIGNAL(clicked(bool)), this, SLOT(OnPlayPause(bool)));
	
	connect(ui.actCapture, SIGNAL(triggered(bool)), this, SLOT(OnCapture(bool)));
	connect(ui.actConnectMotion, SIGNAL(triggered(bool)), this, SLOT(OnMotionConnect(bool)));
	connect(ui.actFastPlay, SIGNAL(triggered(bool)), this, SLOT(OnFastPlay(bool)));
	connect(ui.actTest, SIGNAL(triggered()), this, SLOT(OnTest()));
	connect(ui.actTry, SIGNAL(triggered()), this, SLOT(OnTry()));
	
	connect(grpSrcType, SIGNAL(triggered(QAction*)), this, SLOT(OnSrcTypeSelected(QAction*)));
	connect(ui.btnFromCam, SIGNAL(clicked()), ui.actFromCam, SLOT(trigger()));
	connect(ui.btnFromPic, SIGNAL(clicked()), ui.actFromPic, SLOT(trigger()));
	connect(ui.btnFromVideo, SIGNAL(clicked()), ui.actFromVideo, SLOT(trigger()));
	
	connect(ui.btnPipeline, SIGNAL(clicked(bool)), this, SLOT(OnPplBtnClicked(bool)));
	connect(ui.cmbPipeline, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCmbIndexChanged(int)));

	connect(ui.sliderSpeed, SIGNAL(valueChanged(int)), ui.spinSpeed, SLOT(setValue(int)));
	connect(ui.spinSpeed, SIGNAL(valueChanged(int)), ui.sliderSpeed, SLOT(setValue(int)));
	connect(ui.spinSpeed, SIGNAL(valueChanged(int)), this, SLOT(OnMotionSpeed(int)));
	OnMotionSpeed(10); //��ʼ�ٶ�

	ui.menu_View->addAction(ui.dockMotion->toggleViewAction());
	ui.menu_View->addAction(ui.dockCmds->toggleViewAction());
	ui.menu_View->addAction(ui.dockCalibr->toggleViewAction());
	ui.menu_View->addAction(ui.dockOutput->toggleViewAction());

	QSettings qs("ui_settings.ini", QSettings::IniFormat, this);
	restoreGeometry(qs.value("mainwindow/geometry").toByteArray());
	restoreState(qs.value("mainwindow/windowstate").toByteArray());

	ui.edtCaliLen->setText(qs.value("calibration/realAB").toString());
	ui.edtCaliZ0->setText(qs.value("calibration/z0").toString());
	ui.edtCaliZe->setText(qs.value("calibration/ze").toString());

	ui.cmbPipeline->setCurrentIndex(qs.value("cmdpipeline/currentindex").toInt());

	vwdb::initdb(ui.textOutput, ui.statusBar);
}

MeasureOne::~MeasureOne()
{
	cam->end();
}

//��ȡ�����ļ�
void MeasureOne::readPipelins(QComboBox *cmb)
{
	cmb->clear();
	QFile file("pipelines.txt");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Can't read the file!" ;
		cmb->addItem("<default>", CN("����ֵ��")); //���ļ�ʱ��Ĭ�Ϸ���
	}
	while (!file.atEnd()) {
		QString line(CN(file.readLine()));
		line = line.trimmed();
		if (line.startsWith("<") && line.endsWith(">")){
			//itemͷ���磺 <sample>
			//��ȡitemͷ�е�����
			QString pplname = line.mid(1, line.length() - 2);
			//��ȡһ��pipeline�����е�ÿһ��
			QString cmds;
			while (1){
				QString cmd(CN(file.readLine()));
				if (cmd.trimmed() == "</>" || file.atEnd()) break; //pipeline��β��ʶ
				cmds.append(cmd);
			}
			//����ComboBox��
			cmb->addItem(pplname, cmds);
		}
	}
}

//�㷨��ˮ��ȷ������
void MeasureOne::savePipelins(QComboBox *cmb)
{
	QFile file("pipelines.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't write the file!";
		QMessageBox::information(this, CN("����"), CN("�����ļ�ʧ��"));
	}
	QTextStream tout(&file);
	for (int i = 0; i < cmb->count(); i++){
		QString pplname = cmb->itemText(i);
		QString cmds = cmb->itemData(i).toString();
		tout << QString("<%1>").arg(pplname) << endl;
		tout << cmds.trimmed() << endl;
		tout << "</>" << endl << endl;
	}
}

//******����������������mutex������ʹ��
QImage MeasureOne::getQimg(ImageObject &imgObj)
{
	QImage qimg;
	cv::Mat mat;

	if (!imgObj.dst.empty())
		mat = imgObj.dst;
	else if (!imgObj.img.empty())
		mat = imgObj.img;
	else
		mat = imgObj.src;

	int n = mat.elemSize(); //ÿ����ֽ���
	QImage::Format format;
	if (n == 3)
		format = QImage::Format_RGB888;
	else if (n == 1)
		format = QImage::Format_Grayscale8;
	else{
		qDebug() << "qimage format error, elemsize=" << n;
		return QImage();
	}

	return QImage(mat.data, mat.cols, mat.rows, mat.cols*n, format);
}

void MeasureOne::timerEvent(QTimerEvent *event)
{
	statusFrameIntvl->setText(NUM_STR(cam->dt) + "ms");
	if (!ui.btnPipeline->isChecked())
		ui.txtPipeline->setPlainText(imgProc.toString(cam->isRunning()));
}

void MeasureOne::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	if (key == Qt::Key_W){
		if (!ui.btnMotionForw->isChecked())
			ui.btnMotionForw->click();
	}
	else if (key == Qt::Key_S){
		if (!ui.btnMotionBack->isChecked())
			ui.btnMotionBack->click();
	}
	else if (key == Qt::Key_A){
		if (!ui.btnMotionLeft->isChecked())
			ui.btnMotionLeft->click();
	}
	else if (key == Qt::Key_D){
		if (!ui.btnMotionRight->isChecked())
			ui.btnMotionRight->click();
	}
	else if (key == Qt::Key_U){
		if (!ui.btnMotionUp->isChecked())
			ui.btnMotionUp->click();
	}
	else if (key == Qt::Key_J){
		if (!ui.btnMotionDown->isChecked())
			ui.btnMotionDown->click();
	}
	else if (key == Qt::Key_Space){
		ui.btnMotionStop->click();
	}
	else
		QMainWindow::keyPressEvent(event);
}

void MeasureOne::keyReleaseEvent(QKeyEvent *event)
{
	int key = event->key();
	if (key == Qt::Key_W){
		if (ui.btnMotionForw->isChecked())
			ui.btnMotionForw->click();
	}
	else if (key == Qt::Key_S){
		if (ui.btnMotionBack->isChecked())
			ui.btnMotionBack->click();
	}
	else if (key == Qt::Key_A){
		if (ui.btnMotionLeft->isChecked())
			ui.btnMotionLeft->click();
	}
	else if (key == Qt::Key_D){
		if (ui.btnMotionRight->isChecked())
			ui.btnMotionRight->click();
	}
	else if (key == Qt::Key_U){
		if (ui.btnMotionUp->isChecked())
			ui.btnMotionUp->click();
	}
	else if (key == Qt::Key_J){
		if (ui.btnMotionDown->isChecked())
			ui.btnMotionDown->click();
	}
	else
		QMainWindow::keyReleaseEvent(event);
}

void MeasureOne::closeEvent(QCloseEvent *event)
{
	QSettings qs("ui_settings.ini", QSettings::IniFormat, this);
	qs.setValue("mainwindow/geometry", saveGeometry());
	qs.setValue("mainwindow/windowstate", saveState());

	qs.setValue("calibration/realAB", ui.edtCaliLen->text());
	qs.setValue("calibration/z0", ui.edtCaliZ0->text());
	qs.setValue("calibration/ze", ui.edtCaliZe->text());

	qs.setValue("cmdpipeline/currentindex", ui.cmbPipeline->currentIndex());

	QMainWindow::closeEvent(event);
}

void MeasureOne::showImg()
{
	ImageObject &imgObj = cam->imgObj;

	//statusFrameIntvl->setText(NUM_STR(cam->dt) + "ms"); //ÿһ֡����ʾЧ������

	//setValueÿ�ζ��ᴥ��change�źţ���ʱ��֪����ô��
	//if (cam->srcType == CameraController::fromVideo) 
	//	ui.sliderProgress->setValue(cam->curpos*ui.sliderProgress->maximum());

	QMutexLocker(&imgObj.mutex);
	//QImage qimg = getQimg(imgObj);
	//if (qimg.width() < 1000) qimg = qimg.scaledToWidth(qimg.width() << 1);
	ui.lblShow->setPixmap(QPixmap::fromImage(getQimg(imgObj)));
}

void MeasureOne::finishCollecting()
{
	ui.actCapture->setChecked(false);
	ui.btnPlayPause->setChecked(false);
	killTimer(tmr);
}

void MeasureOne::OnTest()
{

}

void MeasureOne::OnTry()
{

}

void MeasureOne::OnCapture(bool checked)
{
	if (checked){
		ui.btnPlayPause->setChecked(true);
		cam->start();
		tmr = startTimer(1000);
	}
	else
	{
		cam->end();
	}
}

void MeasureOne::OnPlayPause(bool checked)
{
	cam->isPlaying = checked;
}

void MeasureOne::OnFastPlay(bool checked)
{
	cam->willRest = !checked;
}

void MeasureOne::OnPplBtnClicked(bool checked)
{
	QComboBox *cmb = ui.cmbPipeline;
	if (checked){
		//����༭״̬
		int index = cmb->currentIndex();
		QString curText = cmb->currentText();
		QString itemText = cmb->itemText(index);

		//����������µ����֣��½�һ��pipeline����
		if (curText != itemText){
			//Ѱ�����е�����
			for (index = 0; index < cmb->count(); index++){
				if (curText == cmb->itemText(index)) break;
			}
			//�������û��
			if (index == cmb->count()){
				cmb->addItem(curText);
			}
		}

		ui.txtPipeline->setPlainText(cmb->itemData(index).toString());
		ui.txtPipeline->setToolTip(cmb->itemData(index).toString());
		ui.txtPipeline->setReadOnly(false);
		ui.txtPipeline->setAttribute(Qt::WA_InputMethodEnabled);
	}
	else{
		//���������浱ǰָ��
		QString code = ui.txtPipeline->toPlainText();
		if (code == ""){
			QMessageBox::information(this, "", CN("����ɾ������Ŀ"));
			cmb->removeItem(cmb->currentIndex());
		}
		else{
			bool ok = imgProc.parse(code);
			if (!ok){
				QMessageBox::information(this,
					CN("����ʧ��"), CN("����ʧ�ܣ�\n���޸�ָ�"));
				ui.btnPipeline->setChecked(true);
				return;
			}
			//����ɹ�
			cmb->setItemData(cmb->currentIndex(), code);
		}
		ui.txtPipeline->setReadOnly(true);
		ui.txtPipeline->setPlainText(imgProc.toString(cam->isRunning()));
		ui.txtPipeline->setToolTip(imgProc.toString(cam->isRunning()));
		savePipelins(cmb);
	}
}

void MeasureOne::OnCmbIndexChanged(int index)
{
	QComboBox *cmb = ui.cmbPipeline;
	QString itemData = cmb->itemData(index).toString();

	if (ui.btnPipeline->isChecked()){
		ui.txtPipeline->setPlainText(itemData);
		ui.txtPipeline->setToolTip(itemData);
	}
	else{
		imgProc.parse(itemData);
		ui.txtPipeline->setPlainText(imgProc.toString(cam->isRunning()));
		ui.txtPipeline->setToolTip(imgProc.toString(cam->isRunning()));
	}
}

void MeasureOne::OnSrcTypeSelected(QAction *action)
{
	ui.sliderProgress->setValue(0);
	if (action == ui.actFromPic){
		ui.sliderProgress->setEnabled(false);
		cam->srcType = CameraController::fromPic;
		QString path = QFileDialog::getOpenFileName(this, CN("��ͼƬ"));
		cam->path = path;
	}
	else if (action == ui.actFromVideo){
		ui.sliderProgress->setEnabled(true);
		cam->srcType = CameraController::fromVideo;
		QString path = QFileDialog::getOpenFileName(this, CN("����Ƶ"));
		cam->path = path;
	}
	else if (action == ui.actFromCam){
		ui.sliderProgress->setEnabled(false);
		int n = QInputDialog::getInt(this, CN("ѡ�����"), CN("�����������ţ�"), 1);
		cam->camNum = n;
		cam->srcType = CameraController::fromCam;
	}
	ui.actCapture->trigger();
}


void MeasureOne::on_actLive_triggered()
{
	ui.stackedWidget->setCurrentIndex(0);
}


void MeasureOne::on_actMeasure_triggered()
{
	ui.stackedWidget->setCurrentIndex(1);
}


void MeasureOne::on_actModel_triggered()
{
	ui.stackedWidget->setCurrentIndex(2);
}

void MeasureOne::OnMotionConnect(bool checked)
{
	if (checked){
		motion->connect();
	}
	else{
		motion->unConnect();
	}
}

void MeasureOne::OnMotionUp(bool checked)
{
	if (checked){
		ui.btnMotionDown->setChecked(false);
		motion->go(Z, Negative);
	}
	else{
		motion->stop(Z);
	}
}

void MeasureOne::OnMotionDown(bool checked)
{
	if (checked){
		ui.btnMotionUp->setChecked(false);
		motion->go(Z, Positive);
	}
	else{
		motion->stop(Z);
	}
}

void MeasureOne::OnMotionForw(bool checked)
{
	if (checked){
		ui.btnMotionBack->setChecked(false);
		motion->go(Y, Positive);
	}
	else{
		motion->stop(Y);
	}
}

void MeasureOne::OnMotionBack(bool checked)
{
	if (checked){
		ui.btnMotionForw->setChecked(false);
		motion->go(Y, Negative);
	}
	else{
		motion->stop(Y);
	}

}

void MeasureOne::OnMotionLeft(bool checked)
{
	if (checked){
		ui.btnMotionRight->setChecked(false);
		motion->go(X, Negative);
	}
	else{
		motion->stop(X);
	}

}

void MeasureOne::OnMotionRight(bool checked)
{
	if (checked){
		ui.btnMotionLeft->setChecked(false);
		motion->go(X, Positive);
	}
	else{
		motion->stop(X);
	}

}

void MeasureOne::OnMotionStop(bool checked)
{
	motion->stop();
	ui.btnMotionUp->setChecked(false);
	ui.btnMotionDown->setChecked(false);
	ui.btnMotionForw->setChecked(false);
	ui.btnMotionBack->setChecked(false);
	ui.btnMotionLeft->setChecked(false);
	ui.btnMotionRight->setChecked(false);
}

void MeasureOne::OnMotionSpeed(int speed)
{
	qDebug() << "speed:" << speed;
	motion->setSpeed(speed);

	QString tip;
	int freq = speed * 50;
	if (freq < 1000)
		//tip.sprintf("%dHz", freq);
		tip = QString::number(freq) + "Hz";
	else
		//tip.sprintf("%fkHz", freq);
		tip = QString::number(freq / 1000.0f) + "kHz";

	ui.sliderSpeed->setToolTip(tip);
	ui.spinSpeed->setToolTip(tip);
}

void MeasureOne::OnCaliA()
{
	cali->onA();
}

void MeasureOne::OnCaliB()
{
	cali->onB();
}

void MeasureOne::OnCaliGo()
{
	cali->onCaliGo();
}

void MeasureOne::OnCaliStop()
{
	cali->onCaliStop();
}

void MeasureOne::OnCaliCalc()
{
	float realAB = ui.edtCaliLen->text().toFloat();
	float z0 = ui.edtCaliZ0->text().toFloat();
	float ze = ui.edtCaliZe->text().toFloat();
	cali->onCaliCalc(realAB, z0, ze, cam->imgObj.src.cols);
}
