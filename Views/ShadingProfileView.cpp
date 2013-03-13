#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include "ShadingProfileView.h"

ShadingProfileView::ShadingProfileView()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    graphicsView = new QGraphicsView ();
    graphicsView->setScene(new QGraphicsScene ());
    layout->addWidget(graphicsView);

    QHBoxLayout *childLayout = new QHBoxLayout; layout->addLayout(childLayout);
    QVBoxLayout *childLayout1 = new QVBoxLayout;    childLayout->addLayout(childLayout1);
    QVBoxLayout *childLayout2 = new QVBoxLayout;    childLayout->addLayout(childLayout2);

    inwardExtentWidget = new QSlider(Qt::Horizontal);   inwardExtentWidget->setMinimum(0);  inwardExtentWidget->setMaximum(200);
    outwardExtentWidget = new QSlider(Qt::Horizontal);  outwardExtentWidget->setMinimum(0);  outwardExtentWidget->setMaximum(200);
    inwardHeightWidget = new QSlider(Qt::Horizontal);   inwardHeightWidget->setMinimum(-100);  inwardExtentWidget->setMaximum(100);
    outwardHeightWidget = new QSlider(Qt::Horizontal);  outwardHeightWidget->setMinimum(-100);  outwardExtentWidget->setMaximum(100);
    childLayout1->addWidget(new QLabel("In  Extent (0:200)")); childLayout2->addWidget(inwardExtentWidget);
    childLayout1->addWidget(new QLabel("Out Extent (0:200)")); childLayout2->addWidget(outwardExtentWidget);
    childLayout1->addWidget(new QLabel("In  Height (-100:100)")); childLayout2->addWidget(inwardHeightWidget);
    childLayout1->addWidget(new QLabel("Out Height (-100:100)")); childLayout2->addWidget(outwardHeightWidget);

    connect(inwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(outwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(inwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(outwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));

    m_controlPoint = NULL;
}

void ShadingProfileView::updatePath()
{
    ControlPoint cpt;
    if (m_controlPoint != NULL) cpt = *m_controlPoint;

    inwardExtentWidget->setValue(cpt.attributes[0].extent);
    outwardExtentWidget->setValue(cpt.attributes[1].extent);
    inwardHeightWidget->setValue(cpt.attributes[0].height);
    outwardHeightWidget->setValue(cpt.attributes[1].height);

    graphicsView->scene()->clear();
    QRectF rect = QRectF(5,5, 190, 190); // graphicsView->scene()->sceneRect();
    QPointF top(rect.x() + rect.width()/2, rect.y()),
            bottom(rect.x() + rect.width()/2, rect.y()+rect.height()),
            left(rect.x(), rect.y() + rect.height()/2),
            right(rect.x() + rect.width(), rect.y() + rect.height()/2);
    float ps = 5.0;
    graphicsView->scene()->addEllipse(top.x()-ps/2, top.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(bottom.x()-ps/2, bottom.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(left.x()-ps/2, left.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(right.x()-ps/2, right.y()-ps/2, ps, ps);

    QPainterPath path;
    path.moveTo(top.x(), top.y()); path.lineTo(bottom.x(), bottom.y());
    path.closeSubpath();
    path.moveTo(left.x(), left.y()); path.lineTo(right.x(), right.y());

    graphicsView->scene()->addPath(path, QPen(QColor(79, 106, 25), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin),
                     QColor(122, 163, 39));

    /*path.moveTo(0, 0);
    path.cubicTo(99, 0,  50, 50,  99, 99);
    path.cubicTo(0, 99,  50, 50,  0, 0);*/

    /*QPainter painter(this);
    painter.fillRect(0, 0, 100, 100, Qt::white);
    painter.setPen(QPen(QColor(79, 106, 25), 1, Qt::SolidLine,
                        Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(QColor(122, 163, 39));

    painter.drawPath(path);*/


}

void ShadingProfileView::updateControlPointParameters()
{
    if (m_controlPoint != NULL)
    {
        m_controlPoint->attributes[0].extent = inwardExtentWidget->value();
        m_controlPoint->attributes[1].extent = outwardExtentWidget->value();

        m_controlPoint->attributes[0].height = inwardHeightWidget->value();
        m_controlPoint->attributes[1].height = outwardHeightWidget->value();

        emit controlPointAttributesChanged(m_controlPoint->ref);
    }
}
