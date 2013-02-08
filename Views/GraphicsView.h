#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent);

protected:
    void resizeEvent(QResizeEvent *event);
    
signals:
    
public slots:
    void create_bspline();
    void move_bsplines();
    void loadImage();
    void saveImage();
    void loadCurves();
    void saveCurves();

    void changeControlPointSize(int pointSize);
    
};

#endif // GRAPHICSVIEW_H
