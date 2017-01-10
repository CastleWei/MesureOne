#ifndef MODELWIDGET_H
#define MODELWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <Transform3D.h>

class QOpenGLShaderProgram;

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
	protected slots:
	void teardownGL();
	void update();

private:
	// OpenGL State Information
	QOpenGLBuffer m_vertex;
	QOpenGLVertexArrayObject m_object;
	QOpenGLShaderProgram *m_program;

	// Shader Information
	int u_modelToWorld;
	int u_worldToView;
	QMatrix4x4 m_projection;
	Transform3D m_transform;

	// Private Helpers
	void printVersionInformation();
};

#endif // MODELWIDGET_H
