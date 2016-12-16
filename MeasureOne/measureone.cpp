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

	//ͼ����Դ ��ѡ�˵���
	grpSrcType = new QActionGroup(this);
	grpSrcType->addAction(ui.actFromPic);
	grpSrcType->addAction(ui.actFromVideo);
	grpSrcType->addAction(ui.actFromCam);
	ui.actFromCam->setChecked(true);

	//��ʼ��״̬��
	statusMotion = new QLabel(this);
	// 	statusMotion->setFrameStyle(QFrame::Panel | QFrame::Sunken);
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
		QString line(file.readLine());
		line = line.trimmed();
		if (line.startsWith("<") && line.endsWith(">")){
			//itemͷ���磺 <sample>
			//��ȡitemͷ�е�����
			QString pplname = line.mid(1, line.length() - 2);
			//��ȡһ��pipeline�����е�ÿһ��
			QString cmds;
			while (1){
				QString cmd(file.readLine());
				if (cmd.trimmed() == "</>" || file.atEnd()) break; //pipeline��β��ʶ
				cmds.append(cmd);
			}
			//����ComboBox��
			cmb->addItem(pplname, cmds);
		}
	}
}

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
	//	iTmrFps = startTimer(1000); //ˢ�¼���ʱ��
	//}
	//else
	//{
	//	cam->end();
	//	ui.widgShow->stopRefreshing();
	//	killTimer(iTmrFps);
	//}
}

//�㷨��ˮ��ȷ������
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
		ui.txtPipeline->setReadOnly(false);
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
