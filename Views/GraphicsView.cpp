#include <QResizeEvent>
#include <QGLWidget>
#include "Views/GraphicsView.h"
#include "Views/GLScene.h"

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setViewport(new QGLWidget( QGLFormat(QGL::SampleBuffers  | QGL::DirectRendering)));
    setViewportUpdateMode( QGraphicsView::FullViewportUpdate);
    setScene(new GLScene());
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    if (scene())
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    QGraphicsView::resizeEvent(event);
}
