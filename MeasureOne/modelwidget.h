#ifndef MODELWIDGET_H
#define MODELWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <Transform3D.h>
#include "Camera.h"

class QOpenGLShaderProgram;
class Camera;

class ModelWidget : public QOpenGLWidget,
	protected QOpenGLFunctions
{
	Q_OBJECT

public:
	ModelWidget(QWidget *parent);
	//~ModelWidget();
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

public slots:
	void update();

protected slots:
	void teardownGL();

private:
	// OpenGL State Information
	QOpenGLBuffer m_vertex;
	QOpenGLVertexArrayObject m_object;
	QOpenGLShaderProgram *m_program;

	// Shader Information
	int u_modelToWorld;
	int u_worldToCamera;
	int u_cameraToView;
	QMatrix4x4 m_projection;
	Camera m_camera;
	Transform3D m_transform;

	//mouse and key inputs
	QPoint lastPos;
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

	// Private Helpers
	void printVersionInformation();
};

#endif // MODELWIDGET_H
