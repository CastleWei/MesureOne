#pragma once

#include <QPointF>

class CaliData
{


public:
	float dx = 0.32f, dy = 0.16f, dz = 0.16f;

	CaliData()
	{
	}

	~CaliData()
	{
	}

	//�궨һ���⵶
	QPointF caliLine(int i, float off){
		return QPointF(i*dx, off*dz);
	}
};

