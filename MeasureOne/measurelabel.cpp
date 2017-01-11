#include "measurelabel.h"
#include "vwdb.h"

#include <QMouseEvent>
#include <QPainter>
#include <QBrush>
#include <QMutexLocker>

MeasureLabel::MeasureLabel(QObject *parent)
: QLabel((QWidget*)parent)
{

}

MeasureLabel::~MeasureLabel()
{

}

void MeasureLabel::init(ImageObject *imgObj, CaliData *cali)
{
	this->imgObj = imgObj;
	this->cali = cali;
}

void MeasureLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton){
		QPoint pt = event->pos();
		vwdb::println(QString().sprintf("%d,%d", pt.x(), pt.y()));
		double x, y;
		x = pt.x() / factor;
		y = (this->height() - pt.y()) / factor;
		QString str = CN("�õ�����Ϊ: (%1 mm, %2 mm)").arg(x).arg(y);
		vwdb::showstat(str);
		vwdb::msgbox(str);
	}
}

void MeasureLabel::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.fillRect(painter.window(), Qt::black);
	painter.setPen(QPen(Qt::green, 1));

	if (cali == nullptr || imgObj == nullptr || imgObj->offs == nullptr) return;

	//**** TODO: bug: ��̬�ɼ�ʱ���л�������ҳ�������һ��ʼ���ڲ���ҳ���ᣬԤ����showlblˢ��ʱ������

	QMutexLocker locker(&imgObj->mutex);

	int w = this->width();
	int h = this->height();
	int size = imgObj->offsLen;
	QPointF pe = cali->caliLine(size - 1, imgObj->offs[size - 1]); //���һ����
	factor = w / pe.x(); //����

	//�˷�������7~8ms
	//QPointF p0(0.0f, 0.0f); //����ʱ����һ��
	//for (int i = 0; i < size; i++){
	//	QPointF p = cali->caliLine(i, imgObj->offs[i]); //�궨�������
	//	p *= dx; //���ŵ��ͽ���ȿ�
	//	p.setY(h - p.y()); //������Ϊ���ߣ����·�ת
	//	painter.drawLine(p0, p);
	//	p0 = p;
	//}

	//�˷��Ͽ죬4~6ms
	QPointF *vec = new QPointF[size];
	for (int i = 0; i < size; i++){
		QPointF &p = vec[i];
		p = cali->caliLine(i, imgObj->offs[i]); //�궨�������
		p *= factor; //���ŵ��ͽ���ȿ�
		p.setY(h - p.y()); //������Ϊ���ߣ����·�ת
	}
	painter.drawPolyline(vec, size);
	delete[] vec;
}
