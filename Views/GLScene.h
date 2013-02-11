#ifndef GLSCENE_H
#define GLSCENE_H

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include "../Utilities/ImageUtils.h"
#include "../Curve/BSplineGroup.h"

class GLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    typedef enum SketchMode
    {
        ADD_CURVE_MODE,
        IDLE_MODE
    } SketchMode;


    explicit GLScene(QObject *parent = 0);
    virtual ~GLScene();

    //IO functions
    bool openImage(std::string fname);
    void saveImage(std::string fname);
    bool openCurves(std::string fname);
    void saveCurves(std::string fname);

    //Sketching functions
    void createBSpline();
    int registerPointAtScenePos(QPointF scenePos);
    QPointF sceneToImageCoords(QPointF scenePos);
    QPointF imageToSceneCoords(QPointF imgPos);
    bool pick(const QPoint& _mousePos, unsigned int &_nodeIdx,
               unsigned int& _targetIdx, QPointF* _hitPointPtr = NULL);

    //Drawing functions
    void display();
    void draw_image(cv::Mat &image);
    void draw_control_point(int point_id);
    void draw_spline(int spline_id);
    void adjustDisplayedImageSize();

    cv::Mat& currentImage()
    {
        return m_curImage;
    }

    SketchMode& sketchmode()
    {
        return m_sketchmode;
    }
    
protected:
    void drawForeground(QPainter *painter, const QRectF &rect);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    
public slots:

private:
    cv::Mat m_curImage;
    int m_curSplineIdx;

    SketchMode m_sketchmode;
    double  m_modelview [16];
    double  m_projection [16];
    QList<std::pair<uint, uint> > selectedObjects;

public:
    BSplineGroup m_splineGroup;
    float pointSize;

    QSizeF imSize;
};

void nurbsError(uint errorCode);

#endif // GLSCENE_H
