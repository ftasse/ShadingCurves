#ifndef GLSCENE_H
#define GLSCENE_H

#include <QGraphicsScene>
#include "Utilities/ImageUtils.h"
#include "Curve/BSpline.h"

class GLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GLScene(QObject *parent = 0);
    virtual ~GLScene();

    //Drawing functions
    void drawImage(cv::Mat& image);
    void drawControlPoint();
    void drawCurve();

    //IO functions
    bool openImage(std::string fname);
    void saveImage(std::string fname);
    bool openCurve(std::string fname);
    void saveCurve(std::string fname);

    //Sketching functions
    void createBSpline();
    int registerPointAtScenePos(QPointF scenePos);

    cv::Mat& currentImage()
    {
        return m_curImage;
    }

    std::vector<BSpline>& curves()
    {
        return m_curves;
    }
    
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    
public slots:

private:
    cv::Mat m_curImage;

    std::vector<ControlPoint> m_cpts;
    std::vector<BSpline> m_curves;
    int m_curBsplineIndex;
};

#endif // GLSCENE_H
